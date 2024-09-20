import serial
import pyvolume
import re

# Configura a porta serial (substitua 'COM3' pela sua porta correta)
ser = clearserial.Serial('COM7', 115200)

# Variáveis para armazenar o volume atual e o último volume antes de mutar
volume_percent = 100
last_volume = volume_percent

def set_volume_from_serial():
    global volume_percent, last_volume

    while True:
        if ser.in_waiting > 0:
            # Lê a linha recebida pela serial
            serial_data = ser.readline().decode('utf-8').strip()

            # Tenta encontrar o valor de porcentagem no formato "Slider changed to 35%"
            match = re.search(r'Slider changed to (\d+)%', serial_data)

            if match:
                # Extrai o valor e converte para inteiro
                volume_percent = int(match.group(1))
                
                # Define o volume usando pyvolume
                pyvolume.custom(percent=volume_percent)
                print(f"Volume ajustado para {volume_percent}%")

            # Caso receba o comando "Mute", o volume será zerado e o último valor será armazenado
            elif "Mute" in serial_data:
                last_volume = volume_percent
                pyvolume.custom(percent=0)
                print("Volume mutado (0%)")

            # Caso receba o comando "Unmute", o volume será restaurado ao último valor antes do mute
            elif "Unmute" in serial_data:
                pyvolume.custom(percent=last_volume)
                print(f"Volume restaurado para {last_volume}%")

# Executa a função
set_volume_from_serial()
