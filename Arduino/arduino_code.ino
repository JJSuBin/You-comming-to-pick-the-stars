#include<Wire.h> 

/*자이로 센서로 각도 계산하는데 필요한 변수 선언*/
const int MPU_P = 0x68; // 프로파일 수평용 
const int MPU_C = 0x69; // 안장 좌우 제어용
int16_t AcX_P,AcY_P,AcZ_P, Tmp_P, GyX_P,GyY_P,GyZ_P;
int16_t AcX_C,AcY_C,AcZ_C, Tmp_C, GyX_C,GyY_C,GyZ_C;

float RADIAN_TO_DEGREES ;
float Angle;

int sol1Up = 2;
int sol1Down = 3;
int sol2Up = 4;
int sol2Down = 5;

int pSenTop = A2;
int pSenTopValInitial;
int pSenTopVal;
int pSenTopValDiff;

int pSenBottom = A1; 
int pSenBottomVal; 
float AngleRL;

int start = 1;

float calAngle_P();
float calAngle_C();

void pressTop();
void pressBottom();
void makeCrash(char c);   // 충돌 생성 함수
void returnToStraight(char c);  // 상하 작동 후 직진하는 함수
void movestop(); // 실린더가 멈푸는 동작을 실행하는 함수

void setup() {
  Serial.begin(9600);

  // 솔레노이드를 출력으로 초기화
  pinMode(sol1Up,OUTPUT);
  pinMode(sol1Down,OUTPUT);
  pinMode(sol2Up,OUTPUT);
  pinMode(sol2Down,OUTPUT);

}

void loop() {
 /* 유니티에서 상승하기 전까지 동작 없음 */
  while(true){

    // 상승 신호를 받으면(s) 루프 탈출하여 동작 시작
    char sig= Serial.read();
    if(sig == 'u'){break;}
  }

  /* 처음 시작할 때 붕 떠오르고 조금 내려가는 구간*/
  if(start == 1){
  
    init_P();
    init_C();
    
    while(true){
      digitalWrite(sol1Up, HIGH);
      digitalWrite(sol1Down, LOW);
      digitalWrite(sol2Up, HIGH);
      digitalWrite(sol2Down, LOW); 

      char sig= Serial.read();
      if(sig == 'd') {break;}   // 정상에 도착하면 살짝 하강하는 신호 받음
    }
    
    while(true){
      // 살짝 하강 시작
      digitalWrite(sol1Up, LOW);
      digitalWrite(sol1Down, HIGH);
      digitalWrite(sol2Up, LOW);
      digitalWrite(sol2Down, HIGH);

       char sig= Serial.read();
      if(sig == 's') {break;}   // 정지 신호 받음    
    }
    
    while(true){ 
      // 모든 실린더 정지
      digitalWrite(sol1Up, LOW);
      digitalWrite(sol1Down, LOW);
      digitalWrite(sol2Up, LOW);
      digitalWrite(sol2Down, LOW);

      char sig= Serial.read();
      if(sig == 'c') {break;}   // 컨트롤 시작 신호 받음
    }
  
    start = -1;   // 더는 처음 시작상태가 아님
  }

    
  /* 컨트롤 시작 */
  while(true){

    init_P();
    init_C();
    
   AngleRL = calAngle_C();
   Angle = calAngle_P();
   
    pSenTopVal =  analogRead(pSenTop);   
    pSenBottomVal = analogRead(pSenBottom);   

    /* 상하 제어 */
    if(pSenBottomVal > 300)  { pressBottom(); }    // 상승
    else if(pSenTopVal > 400) { pressTop();    }    // 하강
    else {movestop();} //정지

   /* 좌우 제어 */
    if(AngleRL < -6)           {Serial.println(3);}       // 오른쪽으로 기움
    else if(AngleRL > 2)       {Serial.println(2);}     // 왼쪽으로 기움

     delay(10);  // 유니티로 값 전송 시, 문제가 생기지 않기 위함
 
    char sig= Serial.read(); // 유니티에서 보내는 값을 sig 변수에 저장
    /* 충돌 신호를 받으면 makeCrash() 함수 호출*/
    if(sig == 'o') {
        makeCrash('U');
    }
    //delay(10); 

     /* 종료 신호를 받으면 전체 실린더 하강*/
    if(sig == 'e') {
      // 전체 실린더 하강
      digitalWrite(sol1Up, LOW);
      digitalWrite(sol1Down, HIGH);
      digitalWrite(sol2Up, LOW);
      digitalWrite(sol2Down, HIGH);

      break;
    }
  }
}

/* 위쪽 감압센서가 눌렸을 때 : 하강 */
void pressTop(){
  while(true){
  
    /* 상하 압력이 없을 때(빗자루가 직진할 때), 실린더 정지 */
    pSenTopVal = analogRead(pSenTop);         // 위쪽 감압센서 값 읽기
    AngleRL = calAngle_C();
    
   /* 좌우 제어 */
    if(AngleRL < -6)           {Serial.println(3);}       // 오른쪽으로 기움
    else if(AngleRL > 2)       {Serial.println(2);}     // 왼쪽으로 기움
    
    /* 상하 센서의 값을 읽어 정지했다고 판단*/ 
    if(pSenTopVal <= 80){
      Serial.println(-1);
      delay(10);  // 유니티로 값 전송 시, 문제가 생기지 않기 위함
      
      returnToStraight('D'); // 앞쪽 실린더가 내려간 만큼 뒤쪽 실린더 하강하는 동작

      Angle = calAngle_P();
     
      if(Angle > -0.2 && Angle < 0.2){
        /*하강하고 있던 뒤쪽 실린더 정지*/
        digitalWrite(sol2Up, LOW);
        digitalWrite(sol2Down, LOW);
        
        break;
      } 
    }
    else if(pSenTopVal >= 200){
      Serial.println(0);  // 유니티에 값 전송
     delay(10);  // 유니티로 값 전송 시, 문제가 생기지 않기 위함
      
      char sig= Serial.read();
      /* 충돌 신호를 받으면 makeCrash() 함수 호출*/
      if(sig == 'o') {
          makeCrash('D');
      }
      // 앞쪽 실린더 하강
      digitalWrite(sol1Up, LOW);
      digitalWrite(sol1Down, HIGH);
  
      // 뒤쪽 실린더 하강
      digitalWrite(sol2Up, HIGH);
      digitalWrite(sol2Down, LOW);
    }
  }
} 

/* 아래쪽 감압센서가 눌렸을 때 : 상승 */
void pressBottom(){
  while(true){
  
    /* 상하 압력이 없을 때(빗자루가 직진할 때), 실린더 정지 */
    pSenBottomVal  = analogRead(pSenBottom);   // 아래쪽 감압센서 값 읽기

    AngleRL = calAngle_C();

   /* 좌우 제어 */
    if(AngleRL < -6)           {Serial.println(3);}       // 오른쪽으로 기움
    else if(AngleRL > 2)       {Serial.println(2);}     // 왼쪽으로 기움
    
    /* 상하 센서의 값을 읽어 정지했다고 판단*/ 
    if(pSenBottomVal <= 50){
      Serial.println(-1);
     delay(10);  // 유니티로 값 전송 시, 문제가 생기지 않기 위함
      
    returnToStraight('U'); // 앞쪽 실린더가 올라간 만큼 뒤쪽 실린더 상승하는 동작

      Angle = calAngle_P();
      
      /*뒤쪽 실린더가 상승하다 자이로센서 값이 수평 범위 안에 들어온다면 상승하던 뒤쪽 실린더 정지*/
      if(Angle > -0.2 && Angle < 0.2){
        /*하강하고 있던 뒤쪽 실린더 정지*/
        digitalWrite(sol2Up, LOW);
        digitalWrite(sol2Down, LOW);
        
        break;
      } 
    }
    else if(pSenBottomVal >= 150){
      Serial.println(1);  // 유니티에 값 전송
    delay(10);  // 유니티로 값 전송 시, 문제가 생기지 않기 위함
    
      char sig= Serial.read();
      /* 충돌 신호를 받으면 makeCrash() 함수 호출*/
      if(sig == 'o') {
          makeCrash('U');
      }

      // 앞쪽 실린더 상승
      digitalWrite(sol1Up, HIGH);
      digitalWrite(sol1Down, LOW);
  
      // 뒤쪽 실린더 하강
      digitalWrite(sol2Up, LOW);
      digitalWrite(sol2Down, HIGH);
    }
  }
}

void makeCrash(char c){
    if(c == 'D'){
     // 앞쪽 실린더 상승 & 뒤쪽 실린더 하강
      digitalWrite(sol1Up, HIGH);
      digitalWrite(sol1Down, LOW);
      digitalWrite(sol2Up, LOW);
      digitalWrite(sol2Down, HIGH);
      
      delay(450);  

     // 앞쪽 실린더 하강 & 뒤쪽 실린더 상승
      digitalWrite(sol1Up, LOW);
      digitalWrite(sol1Down, HIGH);
      digitalWrite(sol2Up, HIGH);
      digitalWrite(sol2Down, LOW);
      delay(430);   
    }

    else if(c == 'U'){
      // 앞쪽 실린더 하강 & 뒤쪽 실린더 상승
      digitalWrite(sol1Up, LOW);
      digitalWrite(sol1Down, HIGH);
      digitalWrite(sol2Up, HIGH);
      digitalWrite(sol2Down, LOW);
      
      delay(430);  

     // 앞쪽 실린더 상승 & 뒤쪽 실린더 하강
      digitalWrite(sol1Up, HIGH);
      digitalWrite(sol1Down, LOW);
      digitalWrite(sol2Up, LOW);
      digitalWrite(sol2Down, HIGH);
      delay(430);   
    }

      // 전체 실린더 멈춤
      digitalWrite(sol1Up, LOW);
      digitalWrite(sol1Down, LOW);
      digitalWrite(sol2Up, LOW);
      digitalWrite(sol2Down, LOW);
}

void returnToStraight(char c){
  /*상승하고 있다 수평으로 돌아가기 위해서는 앞쪽 실린더가 상승한 만큼 뒤쪽 실린더를 상승시킨다.*/
  if(c == 'U'){
    // 앞쪽 실린더 멈춤
    digitalWrite(sol1Up, LOW);
    digitalWrite(sol1Down, LOW);
    
    // 뒤쪽 실린더 상승
    digitalWrite(sol2Up, HIGH);
    digitalWrite(sol2Down, LOW); 
  }

  /*하강하고 있다 수평으로 돌아가기 위해서는 앞쪽 실린더가 하강한 만큼 뒤쪽 실린더를 하강시킨다.*/
  else if(c == 'D'){
    // 앞쪽 실린더 멈춤
    digitalWrite(sol1Up, LOW);
    digitalWrite(sol1Down, LOW);

    // 뒤쪽 실린더 하강
    digitalWrite(sol2Up, LOW);
    digitalWrite(sol2Down, HIGH);
  } 

    pSenTopVal =  analogRead(pSenTop);   
    pSenBottomVal = analogRead(pSenBottom);   

    /* 상하 제어 */
    if(pSenBottomVal > 300)  { pressBottom(); }    // 상승
    else if(pSenTopVal > 400) { pressTop();    }    // 하강
}

void init_P(){
   Wire.begin();                   //Wire 라이브러리 초기화
  Wire.beginTransmission(MPU_P);  //MPU_P로 데이터 전송 시작
  Wire.write(0x6B);               // PWR_MGMT_1 register
  Wire.write(0);                  //MPU-6050 시작 모드로
  Wire.endTransmission(true);
}
void init_C(){
   Wire.begin();                   //Wire 라이브러리 초기화
  Wire.beginTransmission(MPU_C);  //MPU_P로 데이터 전송 시작
  Wire.write(0x6B);               // PWR_MGMT_1 register
  Wire.write(0);                  //MPU-6050 시작 모드로
  Wire.endTransmission(true);
}

float calAngle_P(){
  float Angle;
  Wire.beginTransmission(MPU_P);    //데이터 전송시작
  Wire.write(0x3B);               // register 0x3B (ACCEL_XOUT_H), 큐에 데이터 기록
  Wire.endTransmission(false);    //연결유지
    
  Wire.requestFrom(MPU_P,6,true);  //MPU에 데이터 요청
  
  //데이터 한 바이트 씩 읽어서 반환
  AcX_P = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY_P = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ_P = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    
   RADIAN_TO_DEGREES = 180/3.14139;
   Angle = atan(AcY_P/sqrt(pow(AcX_P,2) + pow(AcZ_P,2))) * RADIAN_TO_DEGREES;
   //Angle = atan(AcX_P/sqrt(pow(AcY_P,2) + pow(AcZ_P,2))) * RADIAN_TO_DEGREES;
  return Angle;
}

float calAngle_C(){
  float Angle;
  Wire.beginTransmission(MPU_C);    //데이터 전송시작
  Wire.write(0x3B);               // register 0x3B (ACCEL_XOUT_H), 큐에 데이터 기록
  Wire.endTransmission(false);    //연결유지
    
  Wire.requestFrom(MPU_C,6,true);  //MPU에 데이터 요청
  
  //데이터 한 바이트 씩 읽어서 반환
  AcX_C = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY_C = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ_C = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    
   RADIAN_TO_DEGREES = 180/3.14139;
   Angle = atan(AcY_C/sqrt(pow(AcX_C,2) + pow(AcZ_C,2))) * RADIAN_TO_DEGREES;

  return Angle;
}

void movestop(){
  digitalWrite(sol1Up, LOW);
  digitalWrite(sol1Down, LOW);
  digitalWrite(sol2Up, LOW);
  digitalWrite(sol2Down, LOW);
}
