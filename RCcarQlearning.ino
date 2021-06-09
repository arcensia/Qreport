#include <Servo.h>
#include<stdlib.h>
/////////////핀번호 셋/////////////////
const int SERVO = 2;

const int REN=5; //오른쪽En, PWM
const int LEN=6; //왼쪽En, PWM

const int RMC1=8; //오른쪽 모터 제어선 IN1
const int RMC2=9; //오른쪽 모터 제어선 IN2
const int LMC1=10; //왼쪽 모터 제어선 IN3
const int LMC2=11; //왼쪽 모터 제어선 IN4

const int echoPin = 12;
const int trigPin = 13;

Servo myServo;

//////////////Q러닝////////////////////
#define qSize 9
#define AAction 4
const double mygamma = 0.9;
int initialStates[qSize] = {0, 1, 2, 3, 4, 5, 6, 7, 8};//<-상태를 항상 받는걸로 하자.
// 0 장애물 감지x
// 1 장애물 감지 후(기준점 30cm) -30 
// 2 -20
// 3 -10
// 4 사이
// 5  10
// 6  20
// 7  30
// 8 장애물과의 거리(10cm이하)
// Action 0. 좌회전, 1. 우회전, 2. 전진, 3. 후진
int R[9][4] = { {1, 1, 2, -2},
               {-2, 2, -1, 1},
               {-2, 2, -1, 1},
               {-2, 2, -1, 1},
               {1, 1, -2, -1},
               {2, -1, -1, 1},
               {2, -2, -1, 1},
               {2, -2, -1, 1},
               {-1, -1, -2, 2} };

float Q[qSize][AAction];
void initialize();
void chooseAnAction(int shostate);
int getRandomAction(int upperBound, int lowerBound, int gettState);
float reward(int RcurrentState, int action);
void episode(int initialState);
int nowstates();
int obscheck(int need);
void activateAction(int Actnum);
int maximum(int state, int returnIndexOnly);
void learn();
int stoplearn();



void learn(){
    int nowState = 0;
    int learnstop = 0;
    do{
      learnstop++;
//     nowState = nowstates();
      nowState = rand()%9;
      Serial.println(nowState);
      episode(nowState);
//    } while (stoplearn() != 1);//학습 종료지점
    }while(learnstop<=100);
}

void forward (int MC1,int MC2, int EN){//전진함수
  digitalWrite(EN, LOW);
  digitalWrite(MC1, HIGH);
  digitalWrite(MC2, LOW);
  analogWrite(EN, Carspeed());
}

void reverse (int MC1,int MC2,int EN){//후진함수
  digitalWrite(EN, LOW);
  digitalWrite(MC1, LOW);
  digitalWrite(MC2, HIGH);
  analogWrite(EN, Carspeed());
}

void brake (int MC1,int MC2, int EN){
  digitalWrite(EN, LOW);
  digitalWrite(MC1, LOW);
  digitalWrite(MC2, LOW);
  digitalWrite(EN, LOW);
}

void go(){
  forward(LMC1,LMC2,LEN);
  forward(RMC1,RMC2,REN);
//  Serial.println("go");
//  delay(dtime());
}

void back(){
  reverse(LMC1,LMC2,LEN);
  reverse(RMC1,RMC2,REN);
//  Serial.println("back");
//  delay(dtime());
}

void leftturn(){
  brake(LMC1,LMC2,LEN);
  forward(RMC1,RMC2,REN);
//  Serial.println("left turn");
//  delay(dtime());
}

void rightturn(){
  forward(LMC1,LMC2,LEN);
  brake(RMC1,RMC2,REN);
//  Serial.println("right turn");
//  delay(dtime());
}

void qleftturn(){
  reverse(LMC1,LMC2,LEN);
  forward(RMC1,RMC2,REN);
//  Serial.println("qleft turn");
//  delay(dtime());
}

void qrightturn(){
  forward(LMC1,LMC2,LEN);
  reverse(RMC1,RMC2,REN);
//  Serial.println("qright turn");
//  delay(dtime());
}

void bbrake(){
  digitalWrite(LEN, LOW);
  digitalWrite(LMC1, LOW);
  digitalWrite(LMC2, LOW);
  digitalWrite(LEN,LOW);
  digitalWrite(REN, LOW);
  digitalWrite(RMC1, LOW);
  digitalWrite(RMC2, LOW);
  digitalWrite(REN,LOW);
//  Serial.println("brake");
//  delay(dtime());
}


void checkServo(){
  myServo.write(30);//30도 방향 회전
  delay(500);
  myServo.write(90);//90도 방향 회전
  delay(500);
  myServo.write(120);//120도 방향 회전
  delay(500);
}

int readUlt(int pos){
  long duration, distance;
  myServo.write(pos);//서브모터 방향 회전
  delay(400);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn (echoPin, HIGH);
  distance = 340*duration / 10000 / 2;//"time" to "cm"
  
  Serial.println(distance);
  return distance;
}

// int dtime(){// 행동에 대한 딜레이 타임
//   return 500;
// }

int Carspeed(){//차 속도
  return 200;
}
int limcm(){//장애물 인식 거리
  return 30;
}


void episode(int initialState){//에피소드 진행
  chooseAnAction(initialState);
}

void chooseAnAction(int shostate){//정해진 Action에 대해 1회씩 누적하여 Q값 쌓기
	int possibleAction;
  possibleAction = getRandomAction(AAction, 0,shostate);//Action을 가져와
  if(R[shostate][possibleAction]>=0){
    Q[shostate][possibleAction] = reward(shostate, possibleAction);//모두 적용  
  }
}

int getRandomAction(int upperBound, int lowerBound, int gettState){//최대 - 최소//upperCound == qsize == 9, lowerBound == 0 // 1초당 진행 (1회씩 진행)
	int action;
  action = lowerBound + rand() % AAction;
  //activateAction(action);//학습
//  delay(500);
  bbrake();
  return action;
}

int obscheck(int need){
  int Fobs = 0, Lobs = 0, Robs = 0, Z = 0;
  Fobs = readUlt(90);//in servo pos;
  Robs = readUlt(60);//in servo pos;
  Lobs = readUlt(120);//in servo pos;
  myServo.write(90);
  
  Z = Lobs-Robs;
  if(need == 0){
    return Fobs;
  }else{
    return Z;
  }
}

void activateAction(int Actnum){ //Action 0. 좌회전, 1. 우회전, 2. 전진, 3. 후진
  if(Actnum == 0){
    leftturn();
  }
  else if(Actnum == 1){
    rightturn();
  }
  else if(Actnum == 2){
    go();
  }
  else {
    back();
  }
}

int maximum(int state, int returnIndexOnly){
// if returnIndexOnly = 1, a Q matrix index is returned.
// if returnIndexOnly = 0, a Q matrix element is returned.
	int winner;
	int foundNewWinner;
	int done = 0;
  winner = 0;  
  do {
      foundNewWinner = 0;
      for(int i = 0; i < AAction; i++){//state에 대한 최고값 추출
        if((i < winner) || (i > winner)){     //Avoid self-comparison. // i != winner일 때
          if(Q[state][i] > Q[state][winner]){
            winner = i;
            foundNewWinner = 1; // 최종값을 찾았다.
          }
        }
      } // i

    if(foundNewWinner == 0){//못찾았다면 break
      done = 1; // break
    }
  } while(done == 0);

	if(returnIndexOnly == 1){//1인 경우 winner 리턴, 0인경우 R테이블에 대해 reward값을 더한다.
		return winner;
	}
  else{
		return Q[state][winner];
	}
}


float reward(int RcurrentState,int action) {//축적된 R과, maximum에서의 값 == R + Q(g*maximum)
    return R[RcurrentState][action] + (mygamma * maximum(action, 0));
}

void initialize(){//Q테이블 초기화
	//srand((unsigned)time(0));
    for(int i = 0; i < qSize; i++){ // <- State
        for(int j = 0; j <AAction; j++){// <- Action
            Q[i][j] = 0;
		} // j
	} // i
}

int stoplearn() {
    int stop=0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 4; j++) {
            if (Q[i][j] > 10)
            {
                stop = 1;
                return stop;
            }
        } // j
    } // i
}


void setup(){
  //pinset//
  Serial.begin(9600);
  myServo.attach(SERVO);
  pinMode(LEN, OUTPUT);
  pinMode(REN, OUTPUT);
  pinMode(LMC1, OUTPUT);
  pinMode(LMC2, OUTPUT);
  pinMode(RMC1, OUTPUT);
  pinMode(RMC2, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  brake(LMC1,LMC2, LEN);
  brake(RMC1,RMC2, REN);  

  //Qtable set//
  initialize();
  delay(1000);
  learn();
  Serial.println("done");
  //Print out Q matrix.
  for(int i = 0; i < qSize; i++){
    for(int j = 0; j < AAction; j++){
      Serial.print(Q[i][j]);
      if(j < qSize - 1){
        Serial.print(",");
      }
    } // j
      Serial.println();
  } // i
  Serial.println();
}

void loop(){ //학습 완료된 maximum에 따른 최종값을 실행
  int check = 0;
  int currentState = 0;
  int newState = 0;
  check = readUlt(90);
  if(check<40){
    if(check<20||check>990){
      bbrake();
      delay(500);
    }
    delay(500);
    currentState = nowstates();
    //Perform tests, starting at all initial states.
    newState = 0;
    newState = maximum(currentState, 1);//최선의 값
    // Serial.print(currentState);
    currentState = newState;
    activateAction(currentState); // <- 동작
    delay(500);
  }
  else{
    go();
  }
}


int nowstates() {
  int Fobstacle, Zobs, numstate;
  Fobstacle = obscheck(0);
  Zobs = obscheck(1);
    if (Fobstacle < 10) {
        numstate = 8;
    }
    else if (Fobstacle > 30) {
        numstate = 0;
    }
    else {
        if (Zobs > -30) {
            numstate = 2;
            if (Zobs >= -20) {
                numstate = 3;
                if (Zobs >= -10) {
                    numstate = 4;
                    if (Zobs >= 10) {
                        numstate = 5;
                        if (Zobs >= 20) {
                            numstate = 6;
                            if (Zobs >= 30) {
                                numstate = 7;
                            }
                        }
                    }
                }
            }
        }
        else {
            numstate = 1;
        }
    }
    return numstate;
}
