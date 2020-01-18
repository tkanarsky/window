import flask
from flask import request, Response
import wave
from StringIO import StringIO
import pyaudio
from threading import Thread

p = pyaudio.PyAudio()
chunk = 1024
app = flask.Flask(__name__)
app.config["DEBUG"] = True

def play(data):
    f = StringIO(data)
    f = wave.open(f, 'rb')
    stream = p.open(format = p.get_format_from_width(f.getsampwidth()), channels
            = f.getnchannels(), rate = f.getframerate(), output = True)
    data = f.readframes(chunk)
    while data != '':
        stream.write(data)
        data = f.readframes(chunk)

@app.route('/posts/', methods=['POST'])
def post():
    Thread(target=play, args=[request.data,]).start()
    return Response(200)

app.run(port=8080, host="192.168.43.244")
