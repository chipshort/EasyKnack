package com.vin.easyknack.ui;
import com.vin.easyknack.document.KnackDocumentCodec;

import static org.junit.jupiter.api.Assertions.*;
import java.util.*;
import javax.swing.SwingUtilities;
import org.junit.jupiter.api.Test;
import com.vin.easyknack.audio.*;
import com.vin.easyknack.nodes.*;
import com.vingame.glswing.node.model.*;
import com.vingame.glswing.ui.GLComboBox;

class AudioRoutingTest {
    @Test void mutedMathOnlyPassesItsChosenInputToTheOutput() throws Exception {
        SwingUtilities.invokeAndWait(() -> {
            try (var controller = new WorkspaceController()) {
                var graph = controller.workspace().currentGraph();
                var a = GLNodeId.random(); var b = GLNodeId.random(); var math = GLNodeId.random(); var out = GLNodeId.random();
                var value = new ValueNode(); var additive = new AdditiveOscillatorNode();
                var operation = new MathNode(); var output = new OutputNode();
                graph.edit("Replace example", edit -> {
                    for (var node : graph.snapshot().nodes()) edit.removeNode(node.id());
                    edit.addNode(new GLGraphNode(a, value.registration().id(), value.defaults()));
                    edit.addNode(new GLGraphNode(b, additive.registration().id(), additive.defaults()));
                    edit.addNode(new GLGraphNode(math, operation.registration().id(), operation.defaults()).withProperty(GLNodeState.MUTED, GLNodeValue.of(true)));
                    edit.addNode(new GLGraphNode(out, output.registration().id(), output.defaults()));
                    edit.addConnection(new GLGraphConnection(GLNodeConnectionId.random(), new GLNodeEndpoint(a, ValueNode.OUT.value()), new GLNodeEndpoint(math, MathNode.A.value())));
                    edit.addConnection(new GLGraphConnection(GLNodeConnectionId.random(), new GLNodeEndpoint(b, AdditiveOscillatorNode.OUT.value()), new GLNodeEndpoint(math, MathNode.B.value())));
                    edit.addConnection(new GLGraphConnection(GLNodeConnectionId.random(), new GLNodeEndpoint(math, MathNode.OUT.value()), new GLNodeEndpoint(out, OutputNode.IN.value())));
                });
                var before = graph.snapshot();
                var encoded = controller.audioGraph().replaceAll(" ![0-9a-f]{16} "," ");
                assertFalse(encoded.contains(" math "));
                assertTrue(encoded.contains("deviceout 0 "), "Mute must route A directly; B stays unreachable for native scheduling");
                assertEquals(before, graph.snapshot());
            }
        });
    }
    @Test void selectingDeviceImmediatelyUsesItsLiveVolumeAndKeepsUpdating() throws Exception {
        AudioDeviceVolumesTest.drainRequests();
        SwingUtilities.invokeAndWait(()-> {
            var previous=AudioEndpoints.current();
            var speakers=new AudioEndpoint("out:speakers","Speakers",AudioEndpoint.Type.HARDWARE_OUTPUT,false,true,false);
            try(var controller=new WorkspaceController()) {
                AudioEndpoints.publish(new AudioEndpoints.Snapshot(previous.capabilities(),List.of(AudioEndpoint.DEFAULT,speakers)));
                AudioDeviceVolumes.publish(Map.of("default",new AudioDeviceVolumes.Value(1,false,0),speakers.id(),new AudioDeviceVolumes.Value(.1,false,0)));
                java.util.function.Supplier<com.vingame.glswing.node.GLNode> view=()->controller.workspace().editor().getNodes().stream()
                    .filter(n->n.getTitle().equals("Audio Output")).findFirst().orElseThrow();
                var chooser=(GLComboBox<AudioEndpoint>)view.get().getContents().stream().filter(GLComboBox.class::isInstance).findFirst().orElseThrow();
                assertFalse(chooser.isPopupMatchesComboWidth(),"Device names may extend beyond the node width");
                chooser.setSelectedItem(speakers);
                assertEquals(speakers.reference(),controller.workspace().currentGraph().snapshot().nodes().stream()
                    .filter(n->NodeRegistry.require(n.typeId()) instanceof OutputNode).findFirst().orElseThrow().properties().get(OutputNode.ENDPOINT.value()).toJavaValue());
                var slider=(com.vingame.glswing.ui.GLSlider)view.get().getContents().stream().filter(com.vingame.glswing.ui.GLSlider.class::isInstance).findFirst().orElseThrow();
                assertEquals(10,slider.getValue());
                AudioDeviceVolumes.publish(Map.of(speakers.id(),new AudioDeviceVolumes.Value(.12,false,0)));
                assertEquals(12,slider.getValue());
                assertTrue(AudioDeviceVolumesTest.drainRequests().isEmpty(),"Reading the device level must not write it");
                slider.setValue(0);
                assertEquals(Map.of(speakers.id(),0.0),AudioDeviceVolumesTest.drainRequests());
                slider.setValue(23);
                assertEquals(Map.of(speakers.id(),.23),AudioDeviceVolumesTest.drainRequests());
                assertEquals(chooser.getGLStyle().minimumWidth(),chooser.getPreferredGLSize().width(),.01,
                        "Long device names belong in the popup, not the button width");
                assertTrue(view.get().getPreferredGLSize().width()>=Math.max(view.get().getGLStyle().minimumWidth(),
                        chooser.getPreferredGLSize().width()+view.get().getGLStyle().contentPadding().horizontal()));
            } finally { AudioDeviceVolumes.publish(Map.of()); AudioEndpoints.publish(previous); }
        });
    }
    @Test void keyTargetsSurviveCompositeNavigationAndDocumentReload() throws Exception {
        SwingUtilities.invokeAndWait(()-> {
            try(var controller=new WorkspaceController();var reopened=new WorkspaceController()) {
                var output=controller.workspace().currentGraph().snapshot().nodes().stream()
                    .filter(n->NodeRegistry.require(n.typeId()) instanceof OutputNode).findFirst().orElseThrow();
                controller.setNodeProperty(output.id(),OutputNode.VOLUME_KEYS,GLNodeValue.of(true));
                controller.workspace().editor().setSelectedNodes(controller.workspace().editor().getNodes().subList(1,3));
                controller.actions().extractComposite();
                assertEquals(Set.of("default"),controller.audioVolumeLinks());
                controller.actions().openSelectedComposite();
                assertEquals(Set.of("default"),controller.audioVolumeLinks());
                reopened.openDocument("Volume keys",controller.snapshotDocument());
                assertEquals(Set.of("default"),reopened.audioVolumeLinks());
            }
        });
    }
    @Test void groupMuteAndHiddenFlagsNeverDisableLinkedVolumeTargets()throws Exception{
        SwingUtilities.invokeAndWait(()->{try(var controller=new WorkspaceController()){
            var graph=controller.workspace().currentGraph();
            var output=graph.snapshot().nodes().stream().filter(n->NodeRegistry.require(n.typeId()) instanceof OutputNode).findFirst().orElseThrow();
            controller.setNodeProperty(output.id(),OutputNode.VOLUME_KEYS,GLNodeValue.of(true));
            controller.workspace().editor().setSelectedNodes(controller.workspace().editor().getNodes().subList(1,3));controller.actions().extractComposite();
            var group=graph.snapshot().nodes().stream().filter(n->!(NodeRegistry.find(n.typeId()).orElse(null) instanceof OutputNode)&&NodeRegistry.find(n.typeId()).isEmpty()).findFirst().orElseThrow();
            for(boolean muted:List.of(false,true)){
                graph.edit("Group flags",e->{e.setProperty(group.id(),GLNodeState.MUTED,GLNodeValue.of(muted));e.setProperty(group.id(),GLNodeState.HIDDEN,GLNodeValue.of(true));});
                assertEquals(Set.of("default"),controller.audioVolumeLinks());
            }
        }});
    }
    @Test void mutedInputStaysConnectedAsZeroAndOutputMuteIsLocal() throws Exception {
        SwingUtilities.invokeAndWait(()-> {
            try(var controller=new WorkspaceController()) {
                var graph=controller.workspace().currentGraph();
                var source=GLNodeId.random(); var input=new InputNode();
                var osc=graph.snapshot().nodes().stream().filter(n->NodeRegistry.require(n.typeId()) instanceof OscillatorNode).findFirst().orElseThrow();
                graph.edit("Connect input",edit->{
                    edit.addNode(new GLGraphNode(source,input.registration().id(),input.defaults()).withProperty(GLNodeState.MUTED,GLNodeValue.of(true)));
                    edit.addConnection(new GLGraphConnection(GLNodeConnectionId.random(),new GLNodeEndpoint(source,InputNode.OUT.value()),new GLNodeEndpoint(osc.id(),OscillatorNode.FREQUENCY.value())));
                });
                var before=graph.snapshot();
                String encoded=controller.audioGraph().replaceAll(" ![0-9a-f]{16} "," ");
                assertTrue(encoded.contains("math -1 0.0 -1 0.0 multiply 0"));
                assertFalse(encoded.contains("osc -1 "),"Muted input must not activate the oscillator's fallback frequency");
                assertEquals(before,graph.snapshot(),"Execution transformation must not change visible wires");
                var output=before.nodes().stream().filter(n->NodeRegistry.require(n.typeId()) instanceof OutputNode).findFirst().orElseThrow();
                controller.setNodeProperty(output.id(),OutputNode.VOLUME_KEYS,GLNodeValue.of(true));
                assertEquals(Set.of("default"),controller.audioVolumeLinks());
                graph.edit("Mute output",edit->edit.setProperty(output.id(),GLNodeState.MUTED,GLNodeValue.of(true)));
                assertFalse(controller.audioGraph().replaceAll(" ![0-9a-f]{16} "," ").contains("deviceout"));
                assertEquals(Set.of("default"),controller.audioVolumeLinks(),"Mute must not change the key selection");
            }
        });
    }
    @Test void deviceVolumeUpdatesAreRuntimeOnlyAndShared() throws Exception {
        SwingUtilities.invokeAndWait(()-> {
            try(var controller=new WorkspaceController()) {
                var saved=controller.snapshotDocument();
                AudioDeviceVolumes.publish(Map.of("default",new AudioDeviceVolumes.Value(.37,true,.1)));
                var output=controller.workspace().editor().getNodes().stream().filter(n->n.getTitle().equals("Audio Output")).findFirst().orElseThrow();
                var slider=(com.vingame.glswing.ui.GLSlider)output.getContents().stream().filter(com.vingame.glswing.ui.GLSlider.class::isInstance).findFirst().orElseThrow();
                assertEquals(37,slider.getValue()); assertTrue(slider.getText().contains("muted"));
                assertEquals(com.vingame.glswing.ui.GLSlider.Presentation.BAR, slider.getPresentation());
                assertEquals("%", slider.getUnit());
                assertEquals("37", slider.getValueFormatter().apply(37));
                assertEquals(saved,controller.snapshotDocument());
            } finally { AudioDeviceVolumes.publish(Map.of()); }
        });
    }
    @Test void unavailableSelectionsSurviveDiscoveryUndoAndDocumentRoundTrip() throws Exception {
        SwingUtilities.invokeAndWait(() -> {
            var previous = AudioEndpoints.current();
            try (var controller = new WorkspaceController(); var reopened = new WorkspaceController()) {
                var input = new InputNode(); var id = GLNodeId.random();
                var device = new AudioEndpoint("in:disconnected", "USB Interface", AudioEndpoint.Type.HARDWARE_INPUT, true, true, false);
                controller.workspace().currentGraph().edit("Input", e -> e.addNode(new GLGraphNode(id, input.registration().id(), input.defaults())));
                controller.setNodeProperty(id, InputNode.ENDPOINT, GLNodeValue.of(device.reference()));
                var saved = controller.snapshotDocument();
                java.util.function.Supplier<GLComboBox<?>> chooser = () -> (GLComboBox<?>) controller.workspace().editor().getNodes().stream()
                        .filter(n -> AudioNodeFactory.idOf(n).filter(id::equals).isPresent()).findFirst().orElseThrow()
                        .getContents().stream().filter(GLComboBox.class::isInstance).findFirst().orElseThrow();
                assertTrue(chooser.get().getSelectedItem().toString().contains("unavailable"));
                AudioEndpoints.publish(new AudioEndpoints.Snapshot(previous.capabilities(), List.of(device, AudioEndpoint.DEFAULT)));
                assertEquals("[" + (com.vin.easyknack.app.Platform.isWindows() ? "WASAPI" : "ALSA") + "] USB Interface",
                        chooser.get().getSelectedItem().toString());
                assertEquals(saved, controller.snapshotDocument(), "Discovery must not dirty the document");
                AudioEndpoints.publish(new AudioEndpoints.Snapshot(previous.capabilities(), List.of(AudioEndpoint.DEFAULT)));
                assertTrue(chooser.get().getSelectedItem().toString().contains("unavailable"));
                try {
                    var bytes = new java.io.ByteArrayOutputStream(); var codec = new KnackDocumentCodec();
                    codec.write(saved, bytes);
                    reopened.openDocument("Missing device", codec.read(new java.io.ByteArrayInputStream(bytes.toByteArray()), codec.currentSchemaVersion()));
                } catch (java.io.IOException error) { throw new AssertionError(error); }
                assertEquals(controller.audioGraph(), reopened.audioGraph());
                assertTrue(reopened.audioGraph().replaceAll(" ![0-9a-f]{16} "," ").contains(AudioEndpoint.hex(device.id()) + " copy stereo"));
                controller.history().undo();
                assertTrue(controller.audioGraph().replaceAll(" ![0-9a-f]{16} "," ").contains(AudioEndpoint.hex("system") + " copy stereo"));
                controller.history().redo();
                assertTrue(controller.audioGraph().replaceAll(" ![0-9a-f]{16} "," ").contains(AudioEndpoint.hex(device.id()) + " copy stereo"));
            } finally { AudioEndpoints.publish(previous); }
        });
    }
}


