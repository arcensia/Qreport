#include <Servo.h>
//#define trigPin 10G;
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

void setup(){
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
  Serial.println("go");
//  delay(dtime());
}

void back(){
  reverse(LMC1,LMC2,LEN);
  reverse(RMC1,RMC2,REN);
  Serial.println("back");
//  delay(dtime());
}

void leftturn(){
  brake(LMC1,LMC2, LEN);
  forward(RMC1,RMC2,REN);
  Serial.println("left turn");
//  delay(dtime());
}

void rightturn(){
  brake(RMC1,RMC2, REN);
  forward(LMC1,LMC2,LEN);
  Serial.println("right turn");
//  delay(dtime());
}

void qleftturn(){
  reverse(LMC1,LMC2,LEN);
  forward(RMC1,RMC2,REN);
  Serial.println("qleft turn");
//  delay(dtime());
}

void qrightturn(){
  forward(LMC1,LMC2,LEN);
  reverse(RMC1,RMC2,REN);
  Serial.println("qright turn");
//  delay(dtime());
}

void bbrake(){
  brake(LMC1,LMC2, LEN);
  brake(RMC1,RMC2, REN);
  Serial.println("brake");
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
  delay(600);
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


void checkUlt(float Ultcm){
  if(Ultcm<=limcm()){
    leftturn();//좌회전
  }
  else{
    rightturn();//우회전
  }
}



// int dtime(){// 행동에 대한 딜레이 타임
//   return 500;
// }
int Carspeed(){//차 속도
  return 50;
}
int limcm(){//장애물 인식 거리
  return 30;
}

void loop(){
  // go(); //전지
  // bbrake(); //정지
  // leftturn(); // 좌회전
  // rightturn(); // 우회전
  // back(); // 후진
  bbrake();
  // checkUlt(readUlt(90)); // 초음파 체크
  // checkServo(); // 서브모터 체크
}

//////////////Q러닝////////////////////
#define qSize 9
#define AAction 4
const double mygamma = 0.8;
const int iterations = 10;
//int initialStates[qSize] = {0, 1, 2, 3, 4, 5, 6, 7, 8};//<-상태를 항상 받는걸로 하자. 
//int States = 0; //<-현재 state
// 0 장애물 감지x
// 1 장애물 감지 후(기준점 30cm) -30 
// 2 -20
// 3 -10
// 4 사이
// 5  10
// 6  20
// 7  30
// 8 장애물과의 거리(10cm이하)

// 이걸 state에 따른 값이라고 해야할텐데.
//Action 0. 좌회전, 1. 우회전, 2. 전진, 3. 후진
int R[qSize][AAction] =  {{1, 1, 10, -3},
                  			{-3, 3, -3, 0},
                  			{-2, 2, -3, 0},
                  			{-1, 1, -3, 0},
                  			{0, 0, -3, 0},
                  			{1, -1, -3, 0},
                  			{2, -2, -3, 0},
                  			{3, -3, -3, 0},
                  			{-1, -1, -10, 3}
                  			};

int Q[qSize][qSize];
int currentState;

void episode(int initialState);
void chooseAnAction();
int getRandomAction(int upperBound, int lowerBound);
void initialize();
int maximum(int state, int returnIndexOnly);
int reward(int action);

void setup(){
	int newState;
	initialize();
    //Perform learning trials starting at all initial states.
    for(int j = 0; j < iterations/*10*/; j++){
        for(int i = 0; i < qSize; i++){
            episode(initialStates[i]);//<-?
		} // i
	} // j

  //Print out Q matrix.
  for(int i = 0; i <= (qSize - 1); i++){
    for(int j = 0; j <= (qSize - 1); j++){
      Serial.print(Q[i][j]);
      if(j < qSize - 1){
        Serial.print(",");
      }
    } // j
      Serial.println();
  } // i
  Serial.println();
}

void loop(){ 
  forward();
  int presentState = nowstates();
  int newState; // <-?
	//Perform tests, starting at all initial states.
	for(int i = 0; i < qSize; i++){
    currentState = State;
    newState = 0;
		do {
          newState = maximum(currentState, 1);
          Serial.print(currentState);
          Serial.print("->");
          currentState = newState;
          ledOn(currentState+2); // <- 동작?
        } while(currentState < AAction); //<- 마지막 기준점
    Serial.println(5);
	} // i  
}

void initialize(){//초기화
	//srand((unsigned)time(0));
    for(int i = 0; i < qSize; i++){ // <- State
        for(int j = 0; j <qSize; j++){// <- Action
            Q[i][j] = 0;
		} // j
	} // i
}

void episode(int initialState){//초기 state에 대해
    currentState = initialState;
    //Travel from state to state until goal state is reached.
	do {
      chooseAnAction();//<- 여기서 currentState값이 바뀐다. 따라서 State가 0일 때 끝내는건 어떨까
	} while(currentState == 5);//끝이 5야

    //When currentState = 5, run through the set once more to
    //for convergence.
    for(int i = 0; i < qSize; i++){
        chooseAnAction();
	} // i
}
void chooseAnAction(){
	int possibleAction;

    //Randomly choose a possible action connected to the current state.
    possibleAction = getRandomAction(AAction, 0);

	if(R[currentState][possibleAction] >= 0){ //<- ?
        Q[currentState][possibleAction] = reward(possibleAction);
        currentState = possibleAction;
        ledOn(currentState+2);
	}
}

int getRandomAction(int upperBound, int lowerBound){//최대 - 최소//upperCound == qsize == 9, lowerBound == 0
	int action;
	int choiceIsValid = 0;
    //Randomly choose a possible action connected to the current state.
    do {
        //srand((unsigned)time(NULL))
        //Get a random value between 0 and 4.
        action = lowerBound + rand() % AAction;
        activateAction(action);
        delay(1000);
		if(R[currentState][action] > -1){//<-현재 상태에 대한 엑션
            choiceIsValid = 1;
		}
    } while(choiceIsValid == 0);

    return action;
}

// int nowstates(){
//   int Fobstacle, Zobs;
//   Fobstacle, Zobs= obscheck();
//   if(Fobstacle<=30){
//     if(Zobs<= -40){

//     }
//   }

//   return numstate
// }

void obscheck(){
  int Fobs, Lobs, Robs, Z;
  Fobs = checkUlt(90)//in servo pos;
  Robs = checkUlt(60)//in servo pos;
  Lobs = checkUlt(120)//in servo pos;
  Z = Lobs-Robs;
  return Fobs, Z;
}

void activateAction(int Actnum){ //Action 0. 좌회전, 1. 우회전, 2. 전진, 3. 후진
  if(Actnum == 0){
    leftturn();
  }
  else if(Actnum == 1){
    rightturn();
  }
  else if(Actnum == 2){
    forward();
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
      for(int i = 0; i < qSize; i++){
        if((i < winner) || (i > winner)){     //Avoid self-comparison.
          if(Q[state][i] > Q[state][winner]){
            winner = i;
            foundNewWinner = 1;
          }
        }
      } // i

    if(foundNewWinner == 0){
      done = 1; // break
    }
  } while(done == 0);

	if(returnIndexOnly == 1){
		return winner;
	}
  else{
		return Q[state][winner];
	}
}

int reward(int action){
    return R[currentState][action] + (mygamma * maximum(action, 0));
}





