import serial

serialPort = serial.Serial(
    port="COM6", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
)
serialString = '0' * 52
znak = chr(0xEF)
serialString = znak + znak + znak + znak + serialString[4:]
serialString = serialString.encode('latin1')
working = True

serialStringReply = ""

while working:
    i = 0
    print("Sended message: ")
    for char in serialString:
        print(i, ". ", char)
        i += 1

    serialPort.write(serialString)

    # Read data out of the buffer until a carraige return / new line is found
    print("Replay: ")
    serialStringReply = serialPort.read(20)

    i = 0
    for char in serialStringReply:
        print(i, ". ", char)
        i += 1

    odpowiedz = input("Czy chcesz kontynuować? (tak/nie): ")
    if odpowiedz == "nie":
        working = False