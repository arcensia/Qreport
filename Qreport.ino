#include <Servo.h>
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
const double mygamma = 0.8;
const int iterations = 10;
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
//학습에 대한 brake가 필요
// 이걸 state에 따른 값이라고 해야할텐데.
//Action 0. 좌회전, 1. 우회전, 2. 전진, 3. 후진
int R[qSize][AAction] =  {{1, 1, 10, -3},
                  			{-3, 3, -3, 0},
                  			{-2, 2, -3, 0},
                  			{-1, 1, -3, 0},
                  			{1, 1, -3, 0},
                  			{1, -1, -3, 0},
                  			{2, -2, -3, 0},
                  			{3, -3, -3, 0},
                  			{-1, -1, -10, 3}
                  			};

int Q[qSize][AAction];
void chooseAnAction(int shostate);
int getRandomAction(int upperBound, int lowerBound, int gettState);
float reward(int RcurrentState, int action);
void episode(int initialState);
int nowstates();
int obscheck(int need);
void activateAction(int Actnum);
int maximum(int state, int returnIndexOnly);

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
	int newState;
	initialize();
    //Perform learning trials starting at all initial states.
  
  for (int k = 0; k < sizeof(initialStates) / sizeof(int); k++){
        episode(k);
  }

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

// void setup(){
//   Serial.begin(9600);
//   myServo.attach(SERVO);
//   pinMode(LEN, OUTPUT);
//   pinMode(REN, OUTPUT);
//   pinMode(LMC1, OUTPUT);
//   pinMode(LMC2, OUTPUT);
//   pinMode(RMC1, OUTPUT);
//   pinMode(RMC2, OUTPUT);
//   pinMode(echoPin, INPUT);
//   pinMode(trigPin, OUTPUT);
//   brake(LMC1,LMC2, LEN);
//   brake(RMC1,RMC2, REN);  
// }

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

// void loop(){
//   // go(); //전진
//   // bbrake(); //정지
//   // leftturn(); // 좌회전
//   // rightturn(); // 우회전
//   // back(); // 후진
//   bbrake();
//   // checkUlt(readUlt(90)); // 초음파 체크
//   // checkServo(); // 서브모터 체크
// }

// //////////////Q러닝////////////////////
// #define qSize 9
// #define AAction 4
// const double mygamma = 0.8;
// const int iterations = 10;
// int initialStates[qSize] = {0, 1, 2, 3, 4, 5, 6, 7, 8};//<-상태를 항상 받는걸로 하자. 
// // 0 장애물 감지x
// // 1 장애물 감지 후(기준점 30cm) -30 
// // 2 -20
// // 3 -10
// // 4 사이
// // 5  10
// // 6  20
// // 7  30
// // 8 장애물과의 거리(10cm이하)
// //학습에 대한 brake가 필요
// // 이걸 state에 따른 값이라고 해야할텐데.
// //Action 0. 좌회전, 1. 우회전, 2. 전진, 3. 후진
// int R[qSize][AAction] =  {{1, 1, 10, -3},
//                   			{-3, 3, -3, 0},
//                   			{-2, 2, -3, 0},
//                   			{-1, 1, -3, 0},
//                   			{1, 1, -3, 0},
//                   			{1, -1, -3, 0},
//                   			{2, -2, -3, 0},
//                   			{3, -3, -3, 0},
//                   			{-1, -1, -10, 3}
//                   			};

// int Q[qSize][AAction];

// void episode(int initialState);
// void chooseAnAction();
// int getRandomAction(int upperBound, int lowerBound);
// void initialize();
// int maximum(int state, int returnIndexOnly);
// int reward(int action);
// int nowstates();
// void obscheck(int need);

// void chooseAnAction(int shostate);
// int getRandomAction(int upperBound, int lowerBound, int currentState);
// float reward(int RcurrentState, int action);
// int initialStates[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };//<-상태를 항상 받는걸로 하자. 
// void episode(int initialState);
// int nowstates();
// int obscheck(int need);
// void activateAction(int Actnum);
// int maximum(int state, int returnIndexOnly);

// void setup(){
  
//   Serial.begin(9600);
//   myServo.attach(SERVO);
//   pinMode(LEN, OUTPUT);
//   pinMode(REN, OUTPUT);
//   pinMode(LMC1, OUTPUT);
//   pinMode(LMC2, OUTPUT);
//   pinMode(RMC1, OUTPUT);
//   pinMode(RMC2, OUTPUT);
//   pinMode(echoPin, INPUT);
//   pinMode(trigPin, OUTPUT);
//   brake(LMC1,LMC2, LEN);
//   brake(RMC1,RMC2, REN);  

  
// 	int newState;
// 	initialize();
//     //Perform learning trials starting at all initial states.
  
//   for (int k = 0; k < sizeof(initialStates) / sizeof(int); k++){
//         episode(k);
//   }

//   Serial.println("done");

//   //Print out Q matrix.
//   for(int i = 0; i < qSize; i++){
//     for(int j = 0; j < AAction; j++){
//       Serial.print(Q[i][j]);
//       if(j < qSize - 1){
//         Serial.print(",");
//       }
//     } // j
//       Serial.println();
//   } // i
//   Serial.println();
// }

// void loop(){ //maximum에 따른 최종값을 실행
//   forward();
//   int currentState = nowstates();
//   int newState;
// 	//Perform tests, starting at all initial states.
// 	for (int i = 0; i < 9; i++) {//state마다 작동
//         currentState = i;
//         int newState = 0;
//         newState = maximum(currentState, 1);//최선의 값
//         Serial.print(currentState);
//         currentState = newState;
//         activateAction(currentState); // <- 동작
//         delay(1000);
//         Serial.println("");
//     } // i  
// }

// void initialize(){//Q테이블 초기화
// 	//srand((unsigned)time(0));
//     for(int i = 0; i < qSize; i++){ // <- State
//         for(int j = 0; j <qSize; j++){// <- Action
//             Q[i][j] = 0;
// 		} // j
// 	} // i
// }

void episode(int initialState){//state에 대해    <-?????? 한 루프에 대한 끝 지점을 어떻게 집어넣지?// 각상황에 대해서
  for (int i = 0; i < 10000; i++) {
        chooseAnAction(initialState);
  }
}

void chooseAnAction(int shostate){//방을 옮겼다는 느낌? //1회씩 진행된 값이 축적되어 Q값 쌓기
	int possibleAction;

  //Randomly choose a possible action connected to the current state.
  possibleAction = getRandomAction(AAction, 0,shostate);//랜덤에서 R이 0보다 큰 Action을 가져와
	if (R[shostate][possibleAction] >= 0) { //<- R 값에서 +값이면 Q테이블에 적용 
        Q[shostate][possibleAction] = reward(shostate, possibleAction);
        // currentState = possibleAction;
        // activateAction(currentState);//0보다 큰 값을 실행
    }
}

int getRandomAction(int upperBound, int lowerBound, int gettState){//최대 - 최소//upperCound == qsize == 9, lowerBound == 0 // 1초당 진행 (1회씩 진행)
	int action;
    int choiceIsValid = 0;
    //Randomly choose a possible action connected to the current state.
    do {
        //Get a random value between 0 and 4.
        action = lowerBound + rand() % AAction;
        //activateAction(action);   
        //delay(100); //<-여기서 문제가 생기나?
        if (R[gettState][action] >= 0) {//<-R테이블에서 0 이상이면 break
            break;
        }
    } while (choiceIsValid == 0);
    return action;
}

// 0 장애물 감지x
// 1 장애물 감지 후(기준점 30cm) -30 미만
// 2 -30~-20
// 3 -20~-10
// 4 -10~10
// 5  10~20
// 6  20~30
// 7  30 초과
// 8 장애물과의 거리(10cm이하)

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

int obscheck(int need){
  int Fobs= 0, Lobs = 0, Robs = 0, Z = 0;
  Fobs = readUlt(90);//in servo pos;
  delay(300);
  Robs = readUlt(60);//in servo pos;
  delay(300);
  Lobs = readUlt(120);//in servo pos;
  delay(300);
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

// double reward(int action){
//     return R[currentState][action] + (mygamma * maximum(action, 0));
// }

// int reward(int action){//축적된 R과, maximum에서의 값 == R + Q(g*maximum)
//     return R[currentState][action] + (mygamma * maximum(action, 0));
// }

float reward(int RcurrentState,int action) {//축적된 R과, maximum에서의 값 == R + Q(g*maximum)
    return R[RcurrentState][action] + (mygamma * maximum(action, 0));
}

void initialize(){//Q테이블 초기화
	//srand((unsigned)time(0));
    for(int i = 0; i < qSize; i++){ // <- State
        for(int j = 0; j <qSize; j++){// <- Action
            Q[i][j] = 0;
		} // j
	} // i
}


void loop(){ //maximum에 따른 최종값을 실행
  go();
  int currentState = nowstates();
  int newState;
	//Perform tests, starting at all initial states.
	for (int i = 0; i < 9; i++) {//state마다 작동
        currentState = i;
        int newState = 0;
        newState = maximum(currentState, 1);//최선의 값
        Serial.print(currentState);
        currentState = newState;
        activateAction(currentState); // <- 동작
        delay(1000);
        Serial.println("");
    } // i  
}


