class SIM {
private:
    String Reply;
    bool isEverythingOkay = true;
    HardwareSerial* SerialX;
    //SoftwareSerial* MySerial;
    int RING;
    void checker() {
        while (!SerialX->available());
        this->Reply = this->SerialX->readString();
        if (this->Reply.indexOf("OK") < 0) this->isEverythingOkay = false;
        else this->isEverythingOkay = true;
    }

public:
    SIM(HardwareSerial* X = &Serial2, int RING = -1, bool u2s = false) {
        this->SerialX = X;
        this->RING = RING;

        this->SerialX->begin(9600);
        this->SerialX->println("AT");
        this->checker();

        this->SerialX->println("AT+CSQ");
        this->checker();

        this->SerialX->println("AT+CCID");
        this->checker();

        this->SerialX->println("AT+CREG?");
        this->checker();
    }

    void sendSMS(String number, String message) {
        this->SerialX->println("AT+CMGF=1");
        this->checker();

        this->SerialX->printf("AT+CMGS=\"%s\"\r\n", number.c_str());
        this->checker();

        this->SerialX->print(message.c_str());
        this->checker();

        this->SerialX->write(26);
        this->checker();
    }

    //Calling Stuff
    void call(String Number) {
        this->SerialX->printf("ATD+%s;\r\n", Number.c_str());
        this->checker();
    }
    void hangCall() {
        this->SerialX->println("ATH");
    }
    //GPRS Stuff
    bool StratGPRS() {
        SerialX->println("AT+SAPBR=3,1,\"Contype\",\"GPRS\""); //Set Connection to GPRS 
        this->checker();
        SerialX->println("AT+CSTT=\"Internet\",\"\",\"\""); //Set the APN, username and password.
        this->checker();
        SerialX->println("AT+SAPBR=1,1"); //Enable GPRS
        this->checker();
        SerialX->println("AT+SAPBR=2,1"); //check for ip adress
        this->checker();
        return this->isEverythingOkay;
    }
    String GET(String URL) {
        String data;
        SerialX->println("AT+HTTPINIT"); //Enable HTTP mode
        this->checker();
        SerialX->println("AT+HTTPSSL=1"); //Enable SSL
        this->checker();
        SerialX->println("AT+HTTPPARA=\"CID\",1"); //Setting HTTP bearer profile
        this->checker();
        SerialX->printf("AT+HTTPPARA=\"URL\",\"%s\"\r\n", URL.c_str());//Give URL of website we want to access
        this->checker();
        SerialX->println("AT+HTTPACTION=0"); //Start http GET Session
        this->checker();
        while (this->Reply.indexOf("+HTTPACTION:") < 0) {
            this->checker();
        }
        this->Reply.clear();
        SerialX->println("AT+HTTPREAD"); //Read the content of webpage 
        this->checker();
        data = this->Reply;
        SerialX->println("AT+HTTPTERM"); //Terminate the session
        this->checker();
        return data;
    }
    void POST(String URL, String Data) {
        SerialX->println("AT+HTTPINIT"); //Enable HTTP mode
        this->checker();
        SerialX->println("AT+HTTPSSL=1"); //Enable SSL
        this->checker();
        SerialX->println("AT+HTTPPARA=\"CID\",1"); //Setting HTTP bearer profile
        this->checker();
        SerialX->printf("AT+HTTPPARA=\"URL\",\"%s\"\r\n", URL.c_str());//Give URL of website we want to access
        this->checker();
        SerialX->println("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded"); //set header type
        this->checker();
        SerialX->printf("AT+HTTPDATA=%d,5000\r\n", 0); //delete buffer
        this->checker();
        SerialX->printf("AT+HTTPDATA=%d,5000\r\n", Data.length() + 1); //content length
        this->checker();
        SerialX->println(Data); //content
        this->checker();
        SerialX->println("AT+HTTPACTION=1"); //POST
        this->checker();
        SerialX->println("AT+HTTPREAD"); //Read the content of webpage 
        this->checker();
        SerialX->println("AT+HTTPTERM"); //Terminate the session
        this->checker();
    }

    String getReply() {
        return this->Reply;
    }
    bool getStatus() {
        return this->isEverythingOkay;
    }
    void terminal() {
        while (true) {
            delay(100);
            while (Serial.available())
            {
                this->SerialX->write(Serial.read());//Forward what Serial received to Software Serial Port
            }
            while (this->SerialX->available())
            {
                Serial.write(this->SerialX->read());//Forward what Software Serial received to Serial Port
            }
        }
    }


};
