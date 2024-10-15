from flask import Flask
import os
import pyautogui
import webbrowser
import pythoncom
import comtypes
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume

app = Flask(__name__)

# Função para obter o dispositivo de áudio padrão
def get_audio_volume():
    pythoncom.CoInitialize()  # Inicializa o COM
    devices = AudioUtilities.GetSpeakers()
    interface = devices.Activate(IAudioEndpointVolume._iid_, comtypes.CLSCTX_ALL, None)
    return interface.QueryInterface(IAudioEndpointVolume)

# Rotas para controle de volume
@app.route('/increase_volume', methods=['GET'])
def increase_volume():
    volume = get_audio_volume()
    current_volume = volume.GetMasterVolumeLevelScalar()
    new_volume = min(current_volume + 0.05, 1.0)
    volume.SetMasterVolumeLevelScalar(new_volume, None)
    return f'Volume aumentado para {new_volume * 100:.0f}%'

@app.route('/decrease_volume', methods=['GET'])
def decrease_volume():
    volume = get_audio_volume()
    current_volume = volume.GetMasterVolumeLevelScalar()
    new_volume = max(current_volume - 0.05, 0.0)
    volume.SetMasterVolumeLevelScalar(new_volume, None)
    return f'Volume diminuído para {new_volume * 100:.0f}%'

@app.route('/mute', methods=['GET'])
def mute_volume():
    volume = get_audio_volume()
    volume.SetMute(True, None)
    return 'Volume mutado'

@app.route('/unmute', methods=['GET'])
def unmute_volume():
    volume = get_audio_volume()
    volume.SetMute(False, None)
    return 'Volume desmutado'

# Rotas para controle de música
@app.route('/music/play', methods=['GET'])
def play_music():
    pyautogui.press('playpause')
    return 'Toggled play/pause'

@app.route('/music/next', methods=['GET'])
def next_music():
    pyautogui.press('nexttrack')
    return 'Skipped to next track'

@app.route('/music/previous', methods=['GET'])
def previous_music():
    pyautogui.press('prevtrack')
    return 'Went to previous track'

# Rotas para abrir sites e aplicativos
@app.route('/open/youtube', methods=['GET'])
def open_youtube():
    webbrowser.open('https://www.youtube.com')
    return 'Opened YouTube'

@app.route('/open/gmail', methods=['GET'])
def open_gmail():
    webbrowser.open('https://mail.google.com')
    return 'Opened Gmail'

@app.route('/open/tiktok', methods=['GET'])
def open_tiktok():
    webbrowser.open('https://www.tiktok.com')
    return 'Opened TikTok'

@app.route('/open/chatgpt', methods=['GET'])
def open_chatgpt():
    webbrowser.open('https://chat.openai.com/')
    return 'Opened ChatGPT'

@app.route('/open/github', methods=['GET'])
def open_github():
    webbrowser.open('https://github.com/')
    return 'Opened GitHub'

@app.route('/open/downloads', methods=['GET'])
def open_downloads():
    downloads_path = os.path.expanduser('~/Downloads')
    os.startfile(downloads_path)
    return 'Opened Downloads folder'

@app.route('/open/projets', methods=['GET'])
def open_projets():
    os.startfile('F:/')
    return 'Opened Projets folder'

@app.route('/open/notepad', methods=['GET'])
def open_notepad():
    os.startfile('notepad.exe')
    return 'Opened Notepad'

@app.route('/open/chrome', methods=['GET'])
def open_chrome():
    os.startfile('chrome.exe')
    return 'Opened Chrome'

@app.route('/open/vscode', methods=['GET'])
def open_vscode():
    os.startfile('C:/Path/To/Your/VSCode.exe')  # Ajuste o caminho
    return 'Opened Visual Studio Code'

@app.route('/open/arduino', methods=['GET'])
def open_arduino():
    os.startfile('C:/Path/To/Your/Arduino/Arduino.exe')  # Ajuste o caminho
    return 'Opened Arduino IDE'

@app.route('/shutdown', methods=['GET'])
def shutdown():
    os.system('shutdown /s /t 1')  # Comando para desligar Windows
    return 'Shutting down...'

# Abrir configurações do Windows
@app.route('/open/settings', methods=['GET'])
def open_settings():
    os.system('start ms-settings:')  # Abre a janela de configurações
    return 'Configurações abertas'

# Bloquear o PC
@app.route('/lock', methods=['GET'])
def lock_pc():
    os.system('rundll32.exe user32.dll,LockWorkStation')  # Bloqueia a sessão
    return 'PC bloqueado'

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5100)
