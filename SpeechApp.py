from gi import pygtkcompat
import gi
import webbrowser

gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst
GObject.threads_init()
Gst.init(None)
    
gst = Gst
    
print("Using pygtkcompat and Gst from gi")

pygtkcompat.enable() 
pygtkcompat.enable_gtk(version='3.0')

import gtk

appliances = ["light","door", "air", "heat", "fan", "dimmer"]
actions = ["open", "close", "up", "down", "unlock", "lock", "on", "off"]
alias = ["front", "back", "patio", "side", "bedroom", "bathroom", "kitchen", "upstairs", "downstairs"]
percents = ["zero", "ten", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty","ninety","one hundred", "a hundred", "twenty five", "seventy five"]

class SpeechApp(object):
    """GStreamer/PocketSphinx Demo Application"""
    def __init__(self):
        self.initGui()
        self.initGst()

    def initGui(self):
        self.window = gtk.Window()
        self.window.connect("delete-event", gtk.main_quit)
        self.window.set_default_size(400,300)
        self.window.set_border_width(20)
        vbox = gtk.VBox()
        self.txtbox = gtk.TextBuffer()
        self.text = gtk.TextView(buffer=self.txtbox)
        self.text.set_wrap_mode(gtk.WRAP_WORD)
        vbox.pack_start(self.text)
        self.button = gtk.ToggleButton("Start Speech Recognition")
        self.button.connect('clicked', self.buttonClick)
        vbox.pack_start(self.button, False, False, 5)
        self.window.add(vbox)
        self.window.show_all()

    def initGst(self):
        self.pipeline = gst.parse_launch('autoaudiosrc ! audioconvert ! audioresample '
                                         + '! pocketsphinx name=asr ! fakesink')
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect('message::element', self.myMessage)
	asr = self.pipeline.get_by_name("asr")
	asr.set_property('fsg', 'myGrammar.fsg')
        self.pipeline.set_state(gst.State.PAUSED)

    def myMessage(self, bus, msg):
	msgtype = msg.get_structure().get_name()
        if msgtype != 'pocketsphinx':
            return

        if msg.get_structure().get_value('final'):
            self.outputText(msg.get_structure().get_value('hypothesis'), msg.get_structure().get_value('confidence'))
	    self.parseString(msg.get_structure().get_value('hypothesis'))
            self.pipeline.set_state(gst.State.READY)
            self.button.set_active(False)

    def outputText(self, hyp, confidence):
        self.txtbox.begin_user_action()
        self.txtbox.delete_selection(True, self.text.get_editable())
        self.txtbox.insert_at_cursor(hyp + " \n")
	self.txtbox.insert_at_cursor("\n You said: "+hyp+"\n")
        self.txtbox.end_user_action()

    def parseString(self, hyp):
	app = None
	act = None
	ali = None
	num = None
	urlstr = "http://10.8.0.60:8080/PSAPWebApp/smarthome/"
	
	for i in appliances:
	    if i in hyp:
		app = i
		self.txtbox.insert_at_cursor(app + " \n")
		break
	for j in actions:
	    if j in hyp:
		act = j
		#self.txtbox.insert_at_cursor(act + " \n")
		break
	for k in alias:
	    if k in hyp:
		ali = k
		#self.txtbox.insert_at_cursor(ali + " \n")
		break
	for l in percents:
	    if l in hyp:
		num = l
		#self.txtbox.insert_at_cursor(num + " \n")
		break
	
	if app == "light":
	    urlstr += "switchedlight/kiraAvatar/"
	if app == "dimmer":
	    urlstr += "dimmedlight/kiraAvatar/"
	if ali is not None:
	    urlstr += ali+"/"
	if act is not None:
	    urlstr += act+"/"
	if num is not None:
	    urlstr += num+"/"
	if act is None and ali is None and num is None:
	    self.txtbox.insert_at_cursor(" ")
	    #self.txtbox.insert_at_cursor("Not enough info for a valid command")
	else:
	    self.txtbox.insert_at_cursor("\n")
	    #self.txtbox.insert_at_cursor(urlstr + "\n")
	    #webbrowser.open_new(urlstr)

    def buttonClick(self, button):
        if button.get_active():
            button.set_label("Stop Speech Recognition")
            self.pipeline.set_state(gst.State.PLAYING)
        else:
            button.set_label("Start Speech Recognition")
            self.pipeline.set_state(gst.State.PAUSED)
  
	

app = SpeechApp()
gtk.main()
