//pm2.5全部完成
//功能說明
//可設為氣氛燈由APP控制顏色
//可設為pm2.5模式 會從網路讀取pm2.5資料並轉換成對應顏色
#include <String.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#define red 13
#define green 12
#define blue 15
#define interruptPin 4
const char *ssidAP = "esp-pm25";
const char *passwdAP = "test1234";
//const char* ssidSTA = "dlink-dir-818";
//const char* passwdSTA = "0732@csie";
char ssidSTA2[20] = "";   //0~19
char passwdSTA2[20] = ""; //20~39
String ssid = "";
String passwd = "";

#define MAX_STRING_LEN 30

// Setup vars
char tagStr[MAX_STRING_LEN] = "";
char dataStr[MAX_STRING_LEN] = "";
char tmpStr[MAX_STRING_LEN] = "";
char endTag[3] = {'<', '/', '\0'};
int len;
boolean county_bl = false;
boolean site_bl = false;
String county = "";
String site = "";
int pm25 = 0;
int color = 0;
char county_check[10] = "";
char site_check[10] = "";
int setmode = 0;  //40
int indexval = 0; //41
//42 eeprom color(氣氛燈)
//43 r
//44 g
//45 b
//pm2.5上次顏色值
//46
//47
//48

String R_str = "";
String G_str = "";
String B_str = "";
int R_int = 0;
int G_int = 0;
int B_int = 0;
int save = 0;
int connectedval = 0;
int delay_time = 0;

int interruptval = 0;

String data[][2] = {{"XXX", "XX"}, {"雲林縣", "麥寮"}, {"臺東縣", "關山"}, {"澎湖縣", "馬公"}, {"金門縣", "金門"}, {"連江縣", "馬祖"}, {"南投縣", "埔里"}, {"高雄市", "復興"}, {"新北市", "永和"}, {"南投縣", "竹山"}, {"桃園市", "中壢"}, {"新北市", "三重"}, {"宜蘭縣", "東山"}, {"宜蘭縣", "宜蘭"}, {"臺北市", "陽明"}, {"花蓮縣", "花蓮"}, {"臺東縣", "臺東"}, {"屏東縣", "恆春"}, {"屏東縣", "潮州"}, {"屏東縣", "屏東"}, {"高雄市", "小港"}, {"高雄市", "前鎮"}, {"高雄市", "前金"}, {"高雄市", "左營"}, {"高雄市", "楠梓"}, {"高雄市", "林園"}, {"高雄市", "大寮"}, {"高雄市", "鳳山"}, {"高雄市", "仁武"}, {"高雄市", "橋頭"}, {"高雄市", "美濃"}, {"臺南市", "臺南"}, {"臺南市", "安南"}, {"臺南市", "善化"}, {"臺南市", "新營"}, {"嘉義市", "嘉義"}, {"雲林縣", "臺西"}, {"嘉義縣", "朴子"}, {"嘉義縣", "新港"}, {"雲林縣", "崙背"}, {"雲林縣", "斗六"}, {"南投縣", "南投"}, {"彰化縣", "二林"}, {"彰化縣", "線西"}, {"彰化縣", "彰化"}, {"臺中市", "西屯"}, {"臺中市", "忠明"}, {"臺中市", "大里"}, {"臺中市", "沙鹿"}, {"臺中市", "豐原"}, {"苗栗縣", "三義"}, {"苗栗縣", "苗栗"}, {"苗栗縣", "頭份"}, {"新竹市", "新竹"}, {"新竹縣", "竹東"}, {"新竹縣", "湖口"}, {"桃園市", "龍潭"}, {"桃園市", "平鎮"}, {"桃園市", "觀音"}, {"桃園市", "大園"}, {"桃園市", "桃園"}, {"臺北市", "大同"}, {"臺北市", "松山"}, {"臺北市", "古亭"}, {"臺北市", "萬華"}, {"臺北市", "中山"}, {"臺北市", "士林"}, {"新北市", "淡水"}, {"新北市", "林口"}, {"新北市", "菜寮"}, {"新北市", "新莊"}, {"新北市", "板橋"}, {"新北市", "土城"}, {"新北市", "新店"}, {"新北市", "萬里"}, {"新北市", "汐止"}, {"基隆市", "基隆"}};

boolean tagFlag = false;
boolean dataFlag = false;
byte server[] = {223, 200, 80, 138}; // www.weather.gov

// Start ethernet client
WiFiClient client;
WiFiServer webserver(80);

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to initialize
    }
    Serial.println("Serial ready");
    delay(2000);
    EEPROM.begin(512);
    for (int i = 0; i < 20; i++)
    {
        ssidSTA2[i] = EEPROM.read(i);
        //    Serial.println(i);
        delay(50);
    }
    for (int i = 20; i < 40; i++)
    {
        passwdSTA2[i - 20] = EEPROM.read(i);
        //    Serial.println(i);
        delay(50);
    }
    setmode = EEPROM.read(40);
    indexval = EEPROM.read(41);
    save = EEPROM.read(42);
    Serial.print("ssidSTA:");
    Serial.println(ssidSTA2);
    Serial.print("passwdSTA:");
    Serial.println(passwdSTA2);
    pinMode(red, OUTPUT);
    analogWrite(red, 0);
    pinMode(green, OUTPUT);
    analogWrite(green, 0);
    pinMode(blue, OUTPUT);
    analogWrite(blue, 0);
    pinMode(interruptPin, OUTPUT);
    digitalWrite(interruptPin, 0);
    attachInterrupt(interruptPin, interruptFunction, RISING);

    if (setmode == 1)
    {
        WiFi.mode(WIFI_AP_STA);
        //Creat WiFi
        WiFi.softAP(ssidAP, passwdAP);
        delay(500);
        WiFi.begin(ssidSTA2, passwdSTA2); /*
    while (WiFi.status() != WL_CONNECTED) {
      Serial.println(".");
      delay(500);
    }*/
        Serial.println("WiFi connected");
        analogWrite(red, EEPROM.read(46));
        analogWrite(green, EEPROM.read(47));
        analogWrite(blue, EEPROM.read(48));
    }
    else
    {
        WiFi.mode(WIFI_AP);
        //Creat WiFi
        WiFi.softAP(ssidAP, passwdAP);
        delay(500);
        Serial.println("Creat WiFi OK");
        setmode = 0;
        switch (save)
        {
        case 1:
            R_int = 255;
            G_int = 0;
            B_int = 0;
            break;
        case 2:
            R_int = 0;
            G_int = 255;
            B_int = 0;
            break;
        case 3:
            R_int = 0;
            G_int = 0;
            B_int = 255;
            break;
        case 4:
            R_int = 255;
            G_int = 255;
            B_int = 0;
            break;
        case 5:
            R_int = 0;
            G_int = 255;
            B_int = 255;
            break;
        case 6:
            R_int = 255;
            G_int = 0;
            B_int = 255;
            break;
        case 7:
            R_int = 255;
            G_int = 255;
            B_int = 255;
            break;
        case 8:
            R_int = 0;
            G_int = 0;
            B_int = 0;
            break;
        case 9: //紅
            R_int = 255;
            G_int = 0;
            B_int = 0;
            break;
        case 10: //草綠
            R_int = 124;
            G_int = 252;
            B_int = 0;
            break;
        case 11: //藍
            R_int = 0;
            G_int = 0;
            B_int = 255;
            break;
        case 12: //橙
            R_int = 255;
            G_int = 70;
            B_int = 0;
            break;
        case 13: //青
            R_int = 0;
            G_int = 255;
            B_int = 255;
            break;
        case 14: //紫
            R_int = 255;
            G_int = 0;
            B_int = 255;
            break;
        case 15: //讀eeprom的rgb
            R_int = EEPROM.read(43);
            G_int = EEPROM.read(44);
            B_int = EEPROM.read(45);
            break;
        }
        analogWrite(red, R_int);
        analogWrite(green, G_int);
        analogWrite(blue, B_int);
    }

    webserver.begin();
    Serial.println("Start server");
    if (client.connect(server, 80))
    {
        Serial.println("connected");
        client.println("GET /ws/Data/ATM00625/?%24skip=0&%24top=1000&format=xml HTTP/1.0");
        client.println();
        connectedval = 1;
        delay(2000);
    }
    else
    {
        Serial.println("connection failed");
        connectedval = 0;
    }
    data[indexval][0].toCharArray(county_check, 10);
    data[indexval][1].toCharArray(site_check, 10);
    Serial.print("set_county:");
    Serial.println(county_check); //0~19
    Serial.print("set_site:");
    Serial.println(site_check); //20~39
    Serial.print("setmode:");
    Serial.println(setmode); //40
    Serial.print("indexval:");
    Serial.println(indexval); //41
    Serial.print("save:");
    Serial.println(save); //42
    Serial.print("EEPROM R:");
    Serial.println(EEPROM.read(43));
    Serial.print("EEPROM G:");
    Serial.println(EEPROM.read(44));
    Serial.print("EEPROM B:");
    Serial.println(EEPROM.read(45));
    Serial.print("EEPROM PMR:");
    Serial.println(EEPROM.read(46));
    Serial.print("EEPROM PMG:");
    Serial.println(EEPROM.read(47));
    Serial.print("EEPROM PMB:");
    Serial.println(EEPROM.read(48));
}

void loop()
{
    switch (setmode)
    {
    case 0:
    {
        client = webserver.available();
        if (!client)
        {
            return;
        }

        while (!client.available())
        {
            delay(1);
        }

        String req = client.readStringUntil('\r');
        client.flush();

        if (req.indexOf("=") != -1)
        {
            ssid = req.substring(5, req.indexOf("="));
            passwd = req.substring(req.indexOf("=") + 1, req.lastIndexOf("="));

            char ssid2[20];
            char passwd2[20];

            for (int i = 0; i < 20; i++)
                ssid2[i] = ssid[i];
            for (int i = 0; i < 20; i++)
                passwd2[i] = passwd[i];

            Serial.println(ssid2);
            Serial.println(passwd2);

            for (int i = 0; i < 20; i++)
            {
                EEPROM.write(i, ssid2[i]);
                Serial.println(i);
                delay(50);
            }
            for (int i = 20; i < 40; i++)
            {
                EEPROM.write(i, passwd2[i - 20]);
                Serial.println(i);
                delay(50);
            }
            EEPROM.write(40, 1);
            EEPROM.commit();
            Serial.println("EEPROM STA write ok");
            analogWrite(red, 255);
            analogWrite(green, 0);
            analogWrite(blue, 0);
            delay(500);
        }

        if (req.indexOf("index") != -1)
        {
            indexval = req.substring(10, 12).toInt();
            EEPROM.write(41, indexval);
            EEPROM.commit();
            Serial.println("EEPROM indexval write ok");
            analogWrite(red, 0);
            analogWrite(green, 255);
            analogWrite(blue, 0);
            delay(500);
        }

        if (req.indexOf("mode1") != -1)
        {
            EEPROM.write(40, 1);
            EEPROM.commit();
            Serial.println("EEPROM setmode write ok");
            Serial.println("please restart esp8266");
            analogWrite(red, 0);
            analogWrite(green, 0);
            analogWrite(blue, 255);
            delay(500);
        }

        switch (req[5])
        {
        ////指定顏色
        case 'r':
            R_int = 255;
            G_int = 0;
            B_int = 0;
            save = 1;
            break;
        case 'g':
            R_int = 0;
            G_int = 255;
            B_int = 0;
            save = 2;
            break;
        case 'b':
            R_int = 0;
            G_int = 0;
            B_int = 255;
            save = 3;
            break;
        case 'y':
            R_int = 255;
            G_int = 255;
            B_int = 0;
            save = 4;
            break;
        case 'c':
            R_int = 0;
            G_int = 255;
            B_int = 255;
            save = 5;
            break;
        case 'p':
            R_int = 255;
            G_int = 0;
            B_int = 255;
            save = 6;
            break;
        case 'w':
            R_int = 255;
            G_int = 255;
            B_int = 255;
            save = 7;
            break;
        case 'o':
            R_int = 0;
            G_int = 0;
            B_int = 0;
            save = 8;
            break;
        ////指定氣氛顏色
        case 'q': //紅
            R_int = 255;
            G_int = 0;
            B_int = 0;
            save = 9;
            break;
        case 'a': //草綠
            R_int = 125;
            G_int = 255;
            B_int = 0;
            save = 10;
            break;
        case 'z': //藍
            R_int = 0;
            G_int = 0;
            B_int = 255;
            save = 11;
            break;
        case 'x': //橙
            R_int = 255;
            G_int = 70;
            B_int = 0;
            save = 12;
            break;
        case 'e': //青
            R_int = 0;
            G_int = 255;
            B_int = 255;
            save = 13;
            break;
        case 'd': //紫
            R_int = 255;
            G_int = 0;
            B_int = 255;
            save = 14;
            break;
        case 's': //儲存顏色
            EEPROM.write(42, save);
            EEPROM.write(43, R_int);
            EEPROM.write(44, G_int);
            EEPROM.write(45, B_int);
            EEPROM.commit();
            R_int = 0;
            G_int = 0;
            B_int = 0;
            Serial.println("EEPROM color write ok");
            break;
        default:
            R_str = req.substring(5, 8);
            G_str = req.substring(8, 11);
            B_str = req.substring(11, 14);
            R_int = R_str.toInt();
            G_int = G_str.toInt();
            B_int = B_str.toInt();
            save = 15;
            break;
        }
        analogWrite(red, R_int);
        analogWrite(green, G_int);
        analogWrite(blue, B_int);

        client.flush();

        String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

        client.print(s);
        delay(100);
    }
    break;
    case 1:
    {
        while (client.available())
        {
            serialEvent();
        }

        switch (color)
        {
        case 1:
            R_int = 125;
            G_int = 255;
            B_int = 0;
            break;
        case 2:
            R_int = 255;
            G_int = 255;
            B_int = 0;
            break;
        case 3:
            R_int = 255;
            G_int = 70;
            B_int = 0;
            break;
        case 4:
            R_int = 255;
            G_int = 0;
            B_int = 0;
            break;
        case 5:
            R_int = 255;
            G_int = 0;
            B_int = 255;
            break;
        }
        analogWrite(red, R_int);
        analogWrite(green, G_int);
        analogWrite(blue, B_int);
        Serial.print("color:");
        Serial.println(color);

        if (!client.connected())
        {
            //Serial.println();
            //Serial.println("Disconnected");
            client.stop();

            // Time until next update

            if (connectedval == 1)
            {
                delay_time = 1800000;
                EEPROM.write(46, R_int);
                EEPROM.write(47, G_int);
                EEPROM.write(48, B_int);
                EEPROM.commit();
                Serial.println("PM25 write OK");
            }
            else if (connectedval = 0)
                delay_time = 60000;

            Serial.print("Waiting:");
            Serial.println(delay_time);
            delay(delay_time); // 1 minute

            if (client.connect(server, 80))
            {
                Serial.println("Reconnected");
                client.println("GET /ws/Data/ATM00625/?%24skip=0&%24top=1000&format=xml HTTP/1.0");
                client.println();
                connectedval = 1;
                delay(2000);
            }
            else
            {
                Serial.println("Reconnect failed");
                connectedval = 0;
            }
        }
    }
    break;
    }
}

// Process each char from web
void serialEvent()
{

    // Read a char
    char inChar = client.read();
    //Serial.print(".");

    if (inChar == '<')
    {
        addChar(inChar, tmpStr);
        tagFlag = true;
        dataFlag = false;
    }
    else if (inChar == '>')
    {
        addChar(inChar, tmpStr);

        if (tagFlag)
        {
            strncpy(tagStr, tmpStr, strlen(tmpStr) + 1);
        }

        // Clear tmp
        clearStr(tmpStr);

        tagFlag = false;
        dataFlag = true;
    }
    else if (inChar != 10)
    {
        if (tagFlag)
        {
            // Add tag char to string
            addChar(inChar, tmpStr);

            // Check for </XML> end tag, ignore it
            if (tagFlag && strcmp(tmpStr, endTag) == 0)
            {
                clearStr(tmpStr);
                tagFlag = false;
                dataFlag = false;
            }
        }

        if (dataFlag)
        {
            // Add data char to string
            addChar(inChar, dataStr);
        }
    }

    // If a LF, process the line
    if (inChar == 10)
    {
        //      Serial.print("tagStr:");
        //      Serial.println(tagStr);
        //       Serial.print("dataStr:");
        //       Serial.println(dataStr);

        // Find specific tags and print data
        if (matchTag("<Site>"))
        {
            //     Serial.print("Site:");
            //     Serial.print(dataStr);
        }
        if (matchTag("<county>"))
        {
            //     Serial.print(",County:");
            //     Serial.print(dataStr);
        }
        if (matchTag("<PM25>"))
        {
            //   Serial.print(",PM25:");
            //  Serial.print(dataStr);
            //  Serial.println("");
            if ((site_bl == true) && (county_bl == true))
            {
                pm25 = atoi(dataStr);
                Serial.print("county:");
                Serial.print(county);
                Serial.print(",site:");
                Serial.print(site);
                Serial.print(",pm25:");
                Serial.print(pm25);
                Serial.print(",level:");
                switch (pm25)
                {
                case 0 ... 35:
                    color = 1;
                    break;
                case 36 ... 44:
                    color = 2;
                    break;
                case 45 ... 53:
                    color = 3;
                    break;
                case 54 ... 69:
                    color = 4;
                    break;
                case 70 ... 100:
                    color = 5;
                    break;
                }
                Serial.println(color);
                site_bl = false;
                county_bl = false;
            }
        }

        if (strncmp(dataStr, site_check, 5) == 0)
        {
            site_bl = true;
            site = dataStr;
        }
        if (strncmp(dataStr, county_check, 7) == 0)
        {
            county_bl = true;
            county = dataStr;
        }

        // Clear all strings
        clearStr(tmpStr);
        clearStr(tagStr);
        clearStr(dataStr);

        // Clear Flags
        tagFlag = false;
        dataFlag = false;
    }
}

/////////////////////
// Other Functions //
/////////////////////

// Function to clear a string
void clearStr(char *str)
{
    int len = strlen(str);
    for (int c = 0; c < len; c++)
    {
        str[c] = 0;
    }
}

//Function to add a char to a string and check its length
void addChar(char ch, char *str)
{
    char *tagMsg = "<TRUNCATED_TAG>";
    char *dataMsg = "-TRUNCATED_DATA-";

    // Check the max size of the string to make sure it doesn't grow too
    // big.  If string is beyond MAX_STRING_LEN assume it is unimportant
    // and replace it with a warning message.
    if (strlen(str) > MAX_STRING_LEN - 2)
    {
        if (tagFlag)
        {
            clearStr(tagStr);
            strcpy(tagStr, tagMsg);
        }
        if (dataFlag)
        {
            clearStr(dataStr);
            strcpy(dataStr, dataMsg);
        }

        // Clear the temp buffer and flags to stop current processing
        clearStr(tmpStr);
        tagFlag = false;
        dataFlag = false;
    }
    else
    {
        // Add char to string
        str[strlen(str)] = ch;
    }
}

// Function to check the current tag for a specific string
boolean matchTag(char *searchTag)
{
    if (strcmp(tagStr, searchTag) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
void interruptFunction()
{
    interruptval++;
    if (interruptval == 1)
    {
        EEPROM.write(40, 0);
        EEPROM.commit();
        Serial.println("EEPROM setmode write 0");
    }
    if (interruptval == 5)
    {
        for (int i = 0; i < 49; i++)
            EEPROM.write(i, '\0');
        EEPROM.commit();
        Serial.println("EEPROM reset OK");
        analogWrite(red, 255);
        analogWrite(green, 255);
        analogWrite(blue, 255);
    }
    Serial.println("Interrupted");
    digitalWrite(4, 0);
    delay(250);
}