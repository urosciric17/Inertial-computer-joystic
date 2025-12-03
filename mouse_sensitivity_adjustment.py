import serial.tools.list_ports
import time
import os
def clear_screen():
    if(os.name=="nt"):
        os.system("cls")
    else:
        os.system("clear")
ser=None
try:
    ports=serial.tools.list_ports.comports()
    if(len(ports)==0):
        print("Nema serijskih portova!")
        time.sleep(1)
        print("Izlazim...")
        time.sleep(1)
        quit()
    for port in ports:
        ser=serial.Serial(str(port.device),baudrate=9600)
        time.sleep(1)
        print("Konekcija uspostavljena na serijskom portu " +str(port.device))
        time.sleep(2)
        clear_screen()
        while True:
            usr_inpt=input("Unesite ceo broj između 1 i 20 da biste podesili osetljivost\n")
            if(usr_inpt.isdecimal()!=True):
                print("CEO BROJ između 1 i 20!")
                time.sleep(2)
                clear_screen()
                continue
            data_to_send = 5+int(usr_inpt)
            if(data_to_send>25 or data_to_send<6):
                print("Naučite da čitate!")
                time.sleep(2)
                clear_screen()
                continue
            ser.write(data_to_send.to_bytes(1,"little"))
            print("Osetljivost uspešno promenjena!")
            time.sleep(2)
            clear_screen()
except KeyboardInterrupt:
    print("Doviđenja!")
except serial.SerialException:
    print("Nešto nije u redu sa serijskim portom")
    time.sleep(1)
    print("Izlazim...")
    time.sleep(1)
finally:
    if ser is not None and ser.is_open:
        ser.close()