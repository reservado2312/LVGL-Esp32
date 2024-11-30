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

@app.route('/open/chrome', methods=['GET'])
def open_chrome():
    os.startfile('chrome.exe')
    return 'Opened Chrome'

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

@app.route('/open/whatsapp', methods=['GET'])
def open_whatsapp():
    os.startfile('G:\Atalhos\WhatsApp.lnk')
    return 'Opened Whatsapp'

@app.route('/open/steam', methods=['GET'])
def open_steam():
    os.startfile('G:/Atalhos/Steam.lnk')
    return 'Opened Steam'

@app.route('/open/xbox', methods=['GET'])
def open_xbox():
    os.startfile('G:/Atalhos/Xbox.lnk')
    return 'Opened Xbox'

@app.route('/open/vscode', methods=['GET'])
def open_vscode():
    os.startfile('G:/Atalhos/VSCode.lnk')
    return 'Opened VS Code'

@app.route('/open/wamp', methods=['GET'])
def open_wamp():
    os.startfile('G:/Atalhos/WampServer.lnk')
    return 'Opened Wamp Server'

@app.route('/open/arduide', methods=['GET'])
def open_arduide():
    os.startfile('G:/Atalhos/ArduinoIDE.lnk')
    return 'Opened Arduino IDE'

@app.route('/open/calc', methods=['GET'])
def open_calc():
    os.startfile('G:/Atalhos/Calc.lnk')
    return 'Opened Calculator'

@app.route('/open/notepad', methods=['GET'])
def open_notepad():
    os.startfile('G:/Atalhos/Notepad.lnk')
    return 'Opened Notepad'

@app.route('/open/discord', methods=['GET'])
def open_discord():
    os.startfile('G:/Atalhos/Discord.lnk')
    return 'Opened Discord'

@app.route('/open/ytMusic', methods=['GET'])
def open_ytMusic():
    webbrowser.open('https://music.youtube.com/')
    return 'Opened Youtube Music'

@app.route('/open/apMusic', methods=['GET'])
def open_appleMusic():
    webbrowser.open('https://music.apple.com/br/new')
    return 'Opened Apple Music'

@app.route('/open/groove', methods=['GET'])
def open_groove():
    os.startfile('G:/Atalhos/Groove.lnk')
    return 'Opened Groove Musics'

@app.route('/open/Documents', methods=['GET'])
def open_documents():
    os.startfile('G:/Atalhos/Documentos.lnk')
    return 'Opened Documents'

@app.route('/open/Localmusics', methods=['GET'])
def open_Localmusics():
    os.startfile('G:/Atalhos/Musicas.lnk')
    return 'Opened Local Musics'

@app.route('/open/images', methods=['GET'])
def open_images():
    os.startfile('G:/Atalhos/Fotos.lnk')
    return 'Opened Local Images'

@app.route('/open/python', methods=['GET'])
def open_python():
    os.startfile('G:/Atalhos/PythonProjetos.lnk')
    return 'Opened Pythpn Projets'

@app.route('/open/arduino', methods=['GET'])
def open_arduino():
    os.startfile('G:/Atalhos/ArduinoProjetos.lnk')
    return 'Opened Arduino IDE Projects'

@app.route('/open/php', methods=['GET'])
def open_php():
    os.startfile('G:/Atalhos/ProjetosPHP.lnk')
    return 'Opened PHP Projects'

@app.route('/open/videos', methods=['GET'])
def open_videos():
    os.startfile('G:/Atalhos/Videos.lnk')
    return 'Opened Videos Folder'

@app.route('/open/gDrive', methods=['GET'])
def open_gDrive():
    os.startfile('G:/Atalhos/Gdrive.lnk')
    return 'Opened Google Drive Folder'

@app.route('/open/downloads', methods=['GET'])
def open_Downloads():
    os.startfile('G:/Atalhos/Downloads.lnk')
    return 'Opened Downloads Folder'

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5100)
