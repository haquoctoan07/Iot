/*
shiftOut với 8 LED bằng 1 IC HC595
*/
//chân ST_CP (Chân 12) của 74HC595
int latchPin = 8;
//chân SH_CP (Chân 11) của 74HC595
int clockPin = 12;
//Chân DS (chân 14) của 74HC595
int dataPin = 11;
//int VCC=13;

//Trạng thái của LED, hay chính là byte mà ta sẽ gửi qua shiftOut
byte ledStatus;
void setup() {
  //Bạn BUỘC PHẢI pinMode các chân này là OUTPUT
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

void loop() { 
 // digitalWrite(VCC, HIGH);
  //Sáng tuần tự
  ledStatus = 0;//mặc định là không có đèn nào sáng hết (0 = 0b00000000)
  digitalWrite(latchPin, LOW); //các đèn LED sẽ không sáng khi bạn digital LOW
  ledStatus = 0b11111111;
  //ShiftOut ra IC
  shiftOut(dataPin, clockPin, MSBFIRST, ledStatus);  
  digitalWrite(latchPin, HIGH);//các đèn LED sẽ sáng với trạng thái vừa được cập nhập
  delay(100);
    
  digitalWrite(latchPin, LOW); //các đèn LED sẽ không sáng khi bạn digital LOW
  ledStatus = 0;
  //ShiftOut ra IC
  shiftOut(dataPin, clockPin, MSBFIRST, ledStatus);  
  digitalWrite(latchPin, HIGH);//các đèn LED sẽ sáng với trạng thái vừa được cập nhập
  delay(100);
}
