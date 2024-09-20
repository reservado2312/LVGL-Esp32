import pyautogui
import serial
import pyvolume
import re
import time

# Configuração da porta serial (substitua pelo nome da sua porta)
ser = serial.Serial('COM9', 115200)  # Troque 'COM3' pela sua porta

# Variáveis para armazenar o volume atual e o último volume antes de mutar
volume_percent = 100
last_volume = volume_percent
delay = 0.5  # Atraso em segundos para verificar estabilidade do valor

# Função para interpretar os comandos recebidos e acionar os controles de mídia
def controlar_musica(comando):
    if comando == "Prev":
        print("Voltando para a música anterior")
        pyautogui.press('prevtrack')  # Tecla de mídia para música anterior
    elif comando == "Play":
        print("Tocando música")
        pyautogui.press('playpause')  # Tecla de mídia para play/pause
    elif comando == "Pause":
        print("Pausando música")
        pyautogui.press('playpause')  # Mesma tecla para pausar
    elif comando == "Next":
        print("Pulando para a próxima música")
        pyautogui.press('nexttrack')  # Tecla de mídia para próxima música
    else:
        print(f"Comando desconhecido: {comando}")

# Função para ajustar o volume com base nos dados da serial
def ajustar_volume(serial_data):
    global volume_percent, last_volume

    # Tenta encontrar o valor de porcentagem no formato "Slider changed to 35%"
    match = re.search(r'Slider changed to (\d+)%', serial_data)
    if match:
        # Extrai o valor e converte para inteiro
        new_volume = int(match.group(1))
        
        # Aguardar para verificar se o valor está estável
        time.sleep(delay)
        if ser.in_waiting > 0:
            serial_data = ser.readline().decode('utf-8').strip()
            match = re.search(r'Slider changed to (\d+)%', serial_data)
            if match:
                new_volume = int(match.group(1))
            else:
                print("Valor de volume não está mais disponível")
                return
        
        # Atualizar o volume se o valor for estável
        if new_volume != volume_percent:
            volume_percent = new_volume
            pyvolume.custom(percent=volume_percent)
            print(f"Volume ajustado para {volume_percent}%")
    elif "Mute" in serial_data:
        last_volume = volume_percent
        pyvolume.custom(percent=0)
        print("Volume mutado (0%)")
    elif "Unmute" in serial_data:
        pyvolume.custom(percent=last_volume)
        print(f"Volume restaurado para {last_volume}%")

# Loop principal
while True:
    try:
        if ser.in_waiting > 0:
            # Lê a linha recebida pela serial
            serial_data = ser.readline().decode('utf-8').strip()
            
            if "Slider changed to" in serial_data or "Mute" in serial_data or "Unmute" in serial_data:
                ajustar_volume(serial_data)
            else:
                controlar_musica(serial_data)
        
        time.sleep(0.1)  # Pequeno intervalo para evitar sobrecarga
    except Exception as e:
        print(f"Erro: {e}")
        break

