#pragma once
#include "ParameterContracts.hpp"
#include "../FormulaProgram.hpp"
#include "../TriggerInput.hpp"
#include "../BufferRead.hpp"
#include "../Wavetables.hpp"
#include <array>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
namespace easyknack {
struct FormulaNode : NodeProcessor {
    const float* sineTable=nullptr;
    FormulaProgram program;unsigned inputSize=1,outputSize=1,triggerInputs=0,triggerOutputs=0,usedInputs=65535;
    unsigned bufferInputs=0;std::uint64_t position=0,seed=0;bool sampleContext=false;
    std::array<float,16> fallback{};
    std::array<ManualTriggerReceiver,16> buttons;
    using Sample=void(*)(const float*,float*,const FormulaContext*);
    using Block=void(*)(const float* const*,const unsigned*,float* const*,unsigned,const FormulaContext*);
    struct Module {
        Sample sample=nullptr;Block block=nullptr;
#ifdef _WIN32
        HMODULE handle=nullptr;
        ~Module(){if(handle)FreeLibrary(handle);}
#else
        void* handle=nullptr;
        ~Module(){if(handle)dlclose(handle);}
#endif
    };
    std::shared_ptr<Module> module;
    bool supportsControlRate()const noexcept override{return true;}
    bool supportsScalarInputs()const noexcept override{return true;}
    bool supportsEventRate()const noexcept override{return true;}
    bool processesEventBlocks()const noexcept override{return true;}
    void processEventBlock(ProcessContext& c)noexcept override{process(c);}
    bool canShareChannels()const noexcept override{return true;}
    bool supportsSampleFeedback()const noexcept override{return true;}
    bool supportsTriggerFeedback()const noexcept override{return true;}
    char resolveAutoRate(const char* rates,std::size_t count)noexcept override{
        if(sampleContext||triggerOutputs||(triggerInputs&usedInputs))return 'a';
        for(std::size_t i=0;i<std::min<std::size_t>(count,16);i++)if((usedInputs&(1u<<i))&&!(bufferInputs&(1u<<i))&&rates&&rates[i]!='c')return 'a';return 'c';
    }
    ResourceKind inputResourceKind(std::size_t port)const noexcept override{return port<16&&(bufferInputs&(1u<<port))?ResourceKind::history:ResourceKind::none;}
    std::size_t outputCount()const noexcept override{return outputSize;}
    bool writesEventBuffer(std::size_t port)const noexcept override{return (triggerOutputs&(1u<<port))!=0;}
    void bindIdentity(const std::string& id)override{for(unsigned i=0;i<16;i++)buttons[i].bind(id+":in"+std::to_string(i));}
    void shareChannelState(NodeProcessor& p)override{buttons=static_cast<FormulaNode&>(p).buttons;module=static_cast<FormulaNode&>(p).module;}
    void shareVoiceState(NodeProcessor& p)override{shareChannelState(p);}
    void inheritState(const NodeProcessor& other)noexcept override{if(auto* p=dynamic_cast<const FormulaNode*>(&other)){position=p->position;program.inheritState(p->program);for(unsigned i=0;i<16;i++)buttons[i].seen=p->buttons[i].seen;}}
    void configure(std::istream& in)override{
        auto p=parameters::FormulaNodeParameters::read(in);sources=p.sources();
        std::istringstream data(formulaUnhex(p.program_value));std::string version,code,path;
        if(!(data>>version>>inputSize>>outputSize>>triggerInputs>>triggerOutputs)||(version!="FN1"&&version!="FN2")||inputSize>16||outputSize<1||outputSize>8||triggerInputs>65535||triggerOutputs>255)throw std::invalid_argument("Invalid formula ports");
        for(auto& value:fallback)if(!(data>>value)||!std::isfinite(value))throw std::invalid_argument("Invalid formula fallback");
        if(!(data>>code>>path>>usedInputs))throw std::invalid_argument("Missing formula program");
        bufferInputs=0;seed=0;if(version=="FN2"&&(!(data>>bufferInputs>>seed)||bufferInputs>65535||(bufferInputs&triggerInputs)))throw std::invalid_argument("Invalid formula buffer inputs");
        program.read(formulaUnhex(code),inputSize,outputSize);
        static_assert(Wavetables::tableSize==2048);
        bool needsSine=std::any_of(program.code.begin(),program.code.end(),[](const auto& i){return i.op==FormulaProgram::sineWave;})||std::any_of(program.typed.code.begin(),program.typed.code.end(),[](const auto& i){return i.op==TypedFormulaProgram::sineWave;});
        sineTable=needsSine?Wavetables::builtinTables().waves[0][0].data():nullptr;
        sampleContext=program.typed.sampleContext||!program.states.empty()||std::any_of(program.code.begin(),program.code.end(),[](const auto& i){return i.op==FormulaProgram::sampleIndex||i.op==FormulaProgram::samplePosition||i.op==FormulaProgram::rand_||i.op==FormulaProgram::randSeed||i.op==FormulaProgram::bufferRead||i.op==FormulaProgram::bufferTimeline||(i.op==FormulaProgram::bufferInfo&&unsigned(i.value)%4!=3);});
        module.reset();
#ifdef _WIN32
        if(path!="-"){
            auto utf8=formulaUnhex(path);int length=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.data(),int(utf8.size()),nullptr,0);
            if(length>0){std::wstring wide(length,L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.data(),int(utf8.size()),wide.data(),length);
                auto loaded=std::make_shared<Module>();loaded->handle=LoadLibraryW(wide.c_str());
                if(loaded->handle){auto abi=reinterpret_cast<unsigned(*)()>(GetProcAddress(loaded->handle,"easyknack_formula_abi"));
                    loaded->sample=reinterpret_cast<Sample>(GetProcAddress(loaded->handle,"easyknack_formula_sample"));loaded->block=reinterpret_cast<Block>(GetProcAddress(loaded->handle,"easyknack_formula_block"));
                    if(abi&&abi()==6&&loaded->sample&&loaded->block)module=std::move(loaded);
                }
            }
        }
#else
        if(path!="-"){
            auto utf8=formulaUnhex(path);
            auto loaded=std::make_shared<Module>();
            loaded->handle=dlopen(utf8.c_str(),RTLD_NOW|RTLD_LOCAL);
            if(loaded->handle){
                auto abi=reinterpret_cast<unsigned(*)()>(dlsym(loaded->handle,"easyknack_formula_abi"));
                loaded->sample=reinterpret_cast<Sample>(dlsym(loaded->handle,"easyknack_formula_sample"));
                loaded->block=reinterpret_cast<Block>(dlsym(loaded->handle,"easyknack_formula_block"));
                if(abi&&abi()==6&&loaded->sample&&loaded->block)module=std::move(loaded);
            }
        }
#endif
    }
    /** Exported subclasses replace this with the checked expression, without a DLL. */
    virtual void evaluate(const float* input,float* output,const FormulaContext& context)noexcept{if(module)module->sample(input,output,&context);else program.evaluate(input,output,context);}
    FormulaContext contextFor(ProcessContext& c)noexcept{
        FormulaContext context;context.sampleRate=c.sampleRate;context.blockSize=c.blockFrames?c.blockFrames:c.bufferSize;
        context.sampleIndex=c.sampleOffset;context.samplePosition=position+c.sampleOffset;context.seed=seed;context.user=&c;context.state=program.stateValues.data();context.typedState=program.typed.values.data();context.sineTable=sineTable;
        context.inputRate=[](void* user,unsigned port)->float{auto& c=*static_cast<ProcessContext*>(user);return port<c.inputCount&&(c.sourceRates?c.sourceRates[port]!='c':c.input(port)!=nullptr)?1:0;};
        context.bufferInfo=[](void* user,unsigned port,unsigned field,std::uint64_t offset)->float{
            auto* r=static_cast<ProcessContext*>(user)->resource(port);if(!r||r->kind()!=ResourceKind::history)return 0;
            auto& h=*static_cast<const HistoryResource*>(r);double end=bufferHeadAt(h,std::size_t(offset));
            return float(field==0?std::max(0.,end-h.samples.size()):field==3?h.sampleRate:end);
        };
        context.timelineRead=[](void* user,unsigned port,double index,unsigned mode,unsigned interpolation,std::uint64_t offset)->float{
            auto* r=static_cast<ProcessContext*>(user)->resource(port);if(!r||r->kind()!=ResourceKind::history||!std::isfinite(index))return 0;
            auto& h=*static_cast<const HistoryResource*>(r);if(h.samples.empty())return 0;
            double end=bufferHeadAt(h,std::size_t(offset)),begin=std::max(0.,end-h.samples.size()),length=end-begin;if(length<=0)return 0;
            if(mode==1)index=begin+wrapBufferPosition(index-begin,length);else if(mode==0)index=std::clamp(index,begin,end-1);
            if(index<begin-1||index>=end)return 0;
            auto sample=[&](double position)->float{if(mode==1)position=begin+wrapBufferPosition(position-begin,length);else if(mode==0)position=std::clamp(position,begin,end-1);if(position<begin||position>=end)return 0;return h.valueAt(static_cast<std::uint64_t>(position)%h.samples.size(),std::size_t(offset));};
            if(!interpolation)return sample(std::floor(index+.5));
            double base=std::floor(index);float first=sample(base),second=sample(base+1);return first+(second-first)*float(index-base);
        };
        context.bufferSize=[](void* user,unsigned port)->float{auto* r=static_cast<ProcessContext*>(user)->resource(port);return r&&r->kind()==ResourceKind::history?float(static_cast<const HistoryResource*>(r)->samples.size()):0;};
        context.readBuffer=[](void* user,unsigned port,double index,bool wrap,bool interpolate,std::uint64_t offset)->float{
            auto* r=static_cast<ProcessContext*>(user)->resource(port);if(!r||r->kind()!=ResourceKind::history||!std::isfinite(index))return 0;
            auto& h=*static_cast<const HistoryResource*>(r);if(h.samples.empty())return 0;double length=double(h.samples.size());
            if(wrap){index=wrapBufferPosition(index,length);if(!interpolate)index=std::floor(index);return readBufferPosition(h,index,std::size_t(offset)).audio;}
            index=std::clamp(index,0.,length-1);if(!interpolate)index=std::floor(index);
            return readBufferClamped(h,positionBufferTimeline(h,index/length,std::size_t(offset)),std::size_t(offset)).audio;
        };return context;
    }
    void finishContext(const ProcessContext& c)noexcept{auto frames=c.blockFrames?c.blockFrames:c.bufferSize;if(!c.samplewise||c.sampleOffset+c.bufferSize>=frames)position+=frames;}
    void process(ProcessContext& c)noexcept override{
        auto context=contextFor(c);
        if(module&&!triggerInputs&&!triggerOutputs){
            std::array<const float*,16> inputs{};std::array<unsigned,16> strides{};std::array<float*,8> outputs{};
            for(unsigned i=0;i<inputSize;i++){auto v=c.numericInput(i,fallback[i]);inputs[i]=v.data?v.data:&fallback[i];strides[i]=v.data?unsigned(v.stride):0;}
            for(unsigned i=0;i<outputSize;i++){outputs[i]=c.outputAt(i);if(!outputs[i])return;}
            module->block(inputs.data(),strides.data(),outputs.data(),unsigned(c.bufferSize),&context);finishContext(c);return;
        }
        // Fixed stack storage: no allocations or library operations in the callback.
        std::array<std::size_t,16> cursors{};std::array<bool,16> manual{};
        for(unsigned p=0;p<inputSize;p++)if(triggerInputs&(1u<<p)){manual[p]=c.sampleOffset==0&&buttons[p].take()&&!c.input(p)&&!c.events(p);if(auto* e=c.events(p))while(cursors[p]<e->count&&e->events[cursors[p]].sampleOffset<c.sampleOffset)++cursors[p];}
        for(std::size_t frame=0;frame<c.bufferSize;frame++){
            context.sampleIndex=c.sampleOffset+frame;context.samplePosition=position+c.sampleOffset+frame;
            float input[16]{},output[8]{};
            for(unsigned p=0;p<inputSize;p++){
                if(!(triggerInputs&(1u<<p))){input[p]=c.numericInput(p,fallback[p])[frame];continue;}
                bool hit=manual[p]&&frame==0;
                if(auto* e=c.events(p)){while(cursors[p]<e->count&&e->events[cursors[p]].sampleOffset<=c.sampleOffset+frame){auto& event=e->events[cursors[p]++];hit|=event.sampleOffset==c.sampleOffset+frame&&event.kind==ValueEventKind::trigger;}}
                else {auto v=c.numericInput(p)[frame];hit|=std::isfinite(v)&&v!=0;}input[p]=hit?1.f:0.f;
            }
            evaluate(input,output,context);
            for(unsigned p=0;p<outputSize;p++){
                float value=output[p];if(triggerOutputs&(1u<<p)){value=value!=0?1.f:0.f;if(value)if(auto* events=c.eventOutput(p))events->push(unsigned(c.sampleOffset+frame),1,ValueEventKind::trigger);}
                if(auto* destination=c.outputAt(p))destination[frame]=value;
            }
        }
        finishContext(c);
    }
};
EASYKNACK_NODE(FormulaNode,"formula")
}
