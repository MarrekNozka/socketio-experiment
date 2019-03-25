#!/usr/bin/env python3.7
# Soubor:  view.py
# Datum:   25.03.2019 13:11
# Autor:   Marek Nožka, nozka <@t> spseol <d.t> cz
# Licence: GNU/GPL
############################################################################
from . import app, socketio
from flask import (render_template,
                   # Markup,
                   # request,
                   flash,
                   # redirect,
                   # session
                   )
import threading
import serial
serial = serial.Serial('/dev/ttyUSB0')

############################################################################


def read_loop():
    while True:
        cislo = serial.readline()
        print('@@@@@@@@@@@', cislo)
        socketio.emit('input', {'data': cislo.decode('ascii')})


threading._start_new_thread(read_loop, ())


############################################################################

@app.route('/')
def index():
    flash('ahoj')
    return render_template('base.html')


@socketio.on('ahoj')
def ahoj(data=None):
    print(data)


@socketio.on('connected')
def connected(data):
    print('** Conected **')
