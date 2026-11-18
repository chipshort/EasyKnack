package com.vin.easyknack.ui;

import com.vin.easyknack.nodes.Editor;
import com.vingame.glswing.node.model.GLNodeValue;
import com.vingame.glswing.ui.*;
import com.vingame.glswing.layout.GLConstraints;
import com.vingame.glswing.file.*;
import com.vingame.glswing.rendering.GLIcon;
import java.nio.file.*;

/** Inline recording name and file selection; never imports or decodes audio. */
public record RecordingFileEditor() implements Editor {
 public GLNodeValue defaultValue(){return GLNodeValue.of(com.vin.easyknack.app.ApplicationSettings.recordings().resolve("Recording.wav").toAbsolutePath().toString());}
 public Control create(String label,java.util.function.Consumer<GLNodeValue> changed){
  var target=new Target(changed);return new Control(target,target::update);
 }
 static String filename(String text){
  var name=text.strip();
  if(name.isEmpty()||name.equals(".")||name.equals("..")||name.matches(".*[\\\\/:*?\"<>|].*")||name.endsWith(".")||name.endsWith(" "))throw new IllegalArgumentException(AppText.text("recording.invalidName"));
  if(Path.of(name).isAbsolute()||Path.of(name).getNameCount()!=1)throw new IllegalArgumentException(AppText.text("recording.invalidName"));
  var lower=name.toLowerCase(java.util.Locale.ROOT);
  if(!com.vin.easyknack.app.Platform.isWindows()){
   if(lower.endsWith(".mp3"))return name.substring(0,name.length()-4)+".wav";
   return lower.endsWith(".wav")?name:name+".wav";
  }
  return lower.endsWith(".wav")||lower.endsWith(".mp3")?name:name+".wav";
 }
 private static final class Icon {static final GLIcon FOLDER=GLIcon.fromSvgResource("/com/vingame/glswing/icons/filedialog_folder.svg");}
 static final class Target extends GLLayoutGroup {
  final GLTextEditor name=new GLTextEditor();
  final GLButton browse=new GLButton("");
  private Path directory;
  private final java.util.function.Consumer<GLNodeValue> changed;
  Target(java.util.function.Consumer<GLNodeValue> changed){
   this.changed=changed;name.setName(AppText.text("recording.filename"));browse.setName(AppText.text("recording.browse"));
   browse.setIcon(Icon.FOLDER);browse.setToolTipText(AppText.text("recording.browse"));
   addGL(name,GLConstraints.at(0,0).weight(1,1).fillBoth().connectAll());
   addGL(browse,GLConstraints.at(1,0).fillBoth().connectAll());
   name.addActionListener(e->{commit();name.requestGLFocus(null);});browse.addActionListener(e->chooseFile());
   update(GLNodeValue.of(""));sizeButton();
  }
  @Override protected void themeChanged(){super.themeChanged();sizeButton();}
  private void sizeButton(){if(browse!=null){
   browse.setGLStyle(getGLTheme().buttonStyle().withPadding(com.vingame.glswing.layout.GLInsets.all(2)));
   browse.setPreferredGLSize(26,getGLTheme().textEditorStyle().height());
  }}
  void update(GLNodeValue value){
   String text=(String)value.toJavaValue();
   try{var path=text.isBlank()?null:Path.of(text).toAbsolutePath();directory=path==null?null:path.getParent();name.setText(path==null?"Recording.wav":path.getFileName().toString());name.setToolTipText(text.isBlank()?AppText.text("recording.browse"):text);}
   catch(InvalidPathException failure){directory=null;name.setText(text);name.setToolTipText(failure.getMessage());}
  }
  void commit(){
   try{var file=filename(name.getText());name.setText(file);if(directory!=null){var path=directory.resolve(file).toAbsolutePath();changed.accept(GLNodeValue.of(path.toString()));name.setToolTipText(path.toString());}}
   catch(IllegalArgumentException failure){name.setToolTipText(failure.getMessage());}
  }
  GLFileDialogRequest fileRequest(){
   String suggested;
   try{suggested=filename(name.getText());}catch(IllegalArgumentException invalid){suggested="Recording.wav";}
   // Only select a future recording target; collision handling belongs to the recorder.
   var request=GLFileDialogRequest.save().title(AppText.text("recording.browse")).initialDirectory(directory)
    .suggestedFileName(suggested).confirmOverwrite(false);
   if(com.vin.easyknack.app.Platform.isWindows())request.filter("WAV / MP3","wav","mp3");else request.filter("WAV","wav");
   return request.build();
  }
  void selectFile(Path selected){
   var path=selected.toAbsolutePath();directory=path.getParent();name.setText(path.getFileName().toString());commit();
  }
  private void chooseFile(){
   var files=new RememberingFileDialogs();
   files.show("audio.record.directory",fileRequest(),getGLParent())
    .whenComplete((result,error)->java.awt.EventQueue.invokeLater(()->{files.close();if(error!=null){browse.setToolTipText(error.toString());return;}if(result.approved())selectFile(result.selectedPath());}));
  }
 }
}
