import time
import requests
import PySimpleGUI as sg
import numpy as np
import matplotlib.pyplot as plt
from io import BytesIO

# URL of our server
url = "http://localhost:8080"

#initial graphic data
x = []
y = []
counter = 0

# Creation of PySimpleGui window
layout = [
    [sg.Text('State: '), sg.Text('', key='-ERROR-')],
    [sg.Text('Valve value: '), sg.Text('', key='-VALVE-')],
    [sg.Image(key='-IMAGE-')],
    [sg.Slider(range=(0, 100), default_value=0, orientation='h', size=(20, 15), key='-SLIDER-')],
    [sg.Button('Open valve', key='-VALVEBUTTON-')],
    [sg.Button('Go Back')],
    [sg.Button('Exit')]
]

window = sg.Window('Water level graph', layout, finalize=True)
plot_elem = window['-IMAGE-']

# main loop
while True:
    event, values = window.read(timeout=100)  # Timeout = 100ms to not block the execution

    if event == sg.WINDOW_CLOSED or event == 'Exit':
        break

    if event == '-VALVEBUTTON-' or event == 'Go Back':
        # Send slider value to server
        slider_value = values['-SLIDER-']
        response = requests.get(url + "/set_valve_value?value=" + str(slider_value))

    # HTTP Request
    response = requests.get(url)

    # Response state check
    if response.status_code == 200:
        elementi_divisi = response.text.split(';')
        for elemento in elementi_divisi:
            dato = elemento.split(":")
            if dato[0] == "state":
                window['-ERROR-'].update(dato[1])
            elif dato[0] == "water_level":
                y.append(float(dato[1]))
                x.append(counter)
                counter = counter+1
                if len(x) > 20:
                    x.pop(0)
                    y.pop(0)

                # creation of the graph
                plt.plot(x, y, '-o', label='Water label')
                plt.xlabel('last 20 measures')
                plt.ylabel('Level (cm)')
                plt.ylim(0, 1)

                # Saving the image of the graph in the memory
                buf = BytesIO()
                plt.savefig(buf, format='png')
                buf.seek(0)

                # Update of the image in the window
                plot_elem.update(data=buf.read())

                # Cleaning of the image of Matplotlib
                plt.clf()
                buf.close()

            elif dato[0] == "valve_value":
                window['-VALVE-'].update(dato[1])
            else:
                print("Errore nella risposta del server")
    else:
        print(f"Errore nella richiesta HTTP. Codice di stato: {response.status_code}")

# Closing the PySimpleGui window
window.close()
