package com.vin.easyknack.ui;
import static org.junit.jupiter.api.Assertions.*;
import org.junit.jupiter.api.Test;
import java.nio.file.Path;
import com.vingame.glswing.node.model.GLNodeValue;
class RecordingFileEditorTest {
 @Test void inlineRenameKeepsTheSelectedFolderAndUsesAConnectedGroup()throws Exception{
  javax.swing.SwingUtilities.invokeAndWait(()->{
   var values=new java.util.ArrayList<GLNodeValue>();
   var control=new RecordingFileEditor().create("File",values::add);
   var group=(RecordingFileEditor.Target)control.component();
   group.setGLTheme(ApplicationTheme.load(null).theme());
   var original=Path.of("target","recordings","First.wav").toAbsolutePath();control.update().accept(GLNodeValue.of(original.toString()));
   assertEquals("First.wav",group.name.getText());assertNotNull(group.browse.getIcon());
   group.name.setText("Second");group.commit();
   assertEquals(original.resolveSibling("Second.wav").toString(),values.get(0).toJavaValue());
   group.setBounds(0,0,220,24);group.refreshConnections();
   assertEquals(group.name.getX()+group.name.getWidth(),group.browse.getX(),.01);
   assertEquals(0,group.name.getConnectedShape().topRight());
   assertEquals(0,group.browse.getConnectedShape().topLeft());
   group.name.setText("../Wrong");group.commit();assertEquals(1,values.size());
  });
 }
 @Test void enterCommitsTheNameAndLeavesTheTextField()throws Exception{
  javax.swing.SwingUtilities.invokeAndWait(()->{
   var host=new com.vingame.glswing.component.GLSwingPanel(){};
   try{
    var values=new java.util.ArrayList<GLNodeValue>();
    var group=new RecordingFileEditor.Target(values::add);host.addGL(group);
    var original=Path.of("target","First.wav").toAbsolutePath();group.update(GLNodeValue.of(original.toString()));
    host.requestGLFocus(group.name);assertTrue(group.name.isFocused());group.name.setText("Second");
    var event=new java.awt.event.KeyEvent(host,java.awt.event.KeyEvent.KEY_PRESSED,0,0,java.awt.event.KeyEvent.VK_ENTER,'\n');
    for(var listener:host.getKeyListeners())listener.keyPressed(event);
    assertFalse(group.name.isFocused());assertEquals(1,values.size());
    assertEquals(original.resolveSibling("Second.wav").toString(),values.get(0).toJavaValue());
   }finally{host.destroy();}
  });
 }
 @Test void browseSelectsAFileAndSynchronizesTheInlineName()throws Exception{
  javax.swing.SwingUtilities.invokeAndWait(()->{
   var values=new java.util.ArrayList<GLNodeValue>();var group=new RecordingFileEditor.Target(values::add);
   var original=Path.of("target","recordings","First.wav").toAbsolutePath();group.update(GLNodeValue.of(original.toString()));
   group.name.setText("Pending.mp3");var request=group.fileRequest();
   assertEquals(com.vingame.glswing.file.GLFileDialogMode.SAVE,request.mode());
   assertEquals(com.vingame.glswing.file.GLFileSelectionMode.FILES,request.selectionMode());
   boolean windows=com.vin.easyknack.app.Platform.isWindows();
   assertEquals(original.getParent(),request.initialDirectory());
   assertEquals(windows?"Pending.mp3":"Pending.wav",request.suggestedFileName());
   assertFalse(request.confirmOverwrite());
   var selected=Path.of("target","other-recordings",windows?"Chosen.mp3":"Chosen.wav").toAbsolutePath();group.selectFile(selected);
   assertEquals(selected.getFileName().toString(),group.name.getText());assertEquals(selected.toString(),values.get(0).toJavaValue());
   group.name.setText("Renamed.mp3");group.commit();
   assertEquals(selected.resolveSibling(windows?"Renamed.mp3":"Renamed.wav").toString(),values.get(1).toJavaValue());
  });
 }
 @Test void newRecordersUseTheStandardRecordingFolder(){
  assertEquals(com.vin.easyknack.app.ApplicationSettings.recordings().resolve("Recording.wav").toAbsolutePath().toString(),new RecordingFileEditor().defaultValue().toJavaValue());
 }
 @Test void namesKeepTheirFormatAndCannotContainPaths(){
  assertEquals("Take.wav",RecordingFileEditor.filename("Take"));
  assertEquals(com.vin.easyknack.app.Platform.isWindows()?"Take.MP3":"Take.wav",RecordingFileEditor.filename("Take.MP3"));
  assertEquals("Aufnahme ä.wav",RecordingFileEditor.filename("Aufnahme ä.wav"));
  for(String name:java.util.List.of("", "..", "a/b", "a\\b", "bad:name"))
   assertThrows(IllegalArgumentException.class,()->RecordingFileEditor.filename(name));
 }
}
