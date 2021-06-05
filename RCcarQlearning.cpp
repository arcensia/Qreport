#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define AAction 4;
#define qSize 9;

void chooseAnAction(int shostate);
int getRandomAction(int upperBound, int lowerBound, int currentState);
float reward(int RcurrentState, int action);
//int initialStates[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };//<-상태를 항상 받는걸로 하자. 
void episode(int initialState);
int nowstates();
int obscheck(int need);
void activateAction(int Actnum);
int maximum(int state, int returnIndexOnly);
int stoplearn();

int R[9][4] = { {1, 1, 10, -3},
               {-3, 3, -3, 0},
               {-2, 2, -3, 0},
               {-1, 1, -3, 0},
               {1, 1, -3, -1},
               {1, -1, -3, 0},
               {2, -2, -3, 0},
               {3, -3, -3, 0},
               {-1, -1, -10, 3} };
float Q[9][4];
void initialize();

void learn() {
    int nowState = 0;
    int learnstop = 0;
    do
    {
        nowState = nowstates();
        episode(nowState);
        //learnstop++;
        learnstop = stoplearn();
        if (learnstop%10 == 0)
        {
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 4; j++) {
                    printf("%f\t", Q[i][j]);
                    if (j < 4 - 1) {
                        printf(",");
                    }
                } // j
                printf("\n");
            } // i
            printf("\n");
        }
        //} while (10000 != learnstop);//학습 종료지점
    } while (learnstop != 1);//학습 종료지점
}
void setup() {
    initialize();
    learn();

    //Print out Q matrix.
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%f\t", Q[i][j]);
            if (j < 4 - 1) {
                printf(",");
            }
        } // j
        printf("\n");
    } // i
    printf("\n");
}

void episode(int initialState) {//state에 대해    <-?????? 한 루프에 대한 끝 지점을 어떻게 집어넣지?// 각상황에 대해서
    chooseAnAction(initialState);
    printf("\n");
}

void initialize() {//Q테이블 초기화
    for (int i = 0; i < 9; i++) { // <- State
        for (int j = 0; j < 4; j++) {// <- Action
            Q[i][j] = 0;
        } // j
    } // i
}

void chooseAnAction(int shostate) {//방을 옮겼다는 느낌? //1회씩 진행된 값이 축적되어 Q값 쌓기
    int possibleAction;
    //Randomly choose a possible action connected to the current state.
    possibleAction = getRandomAction(4, 0, shostate);//랜덤에서 R이 0보다 큰 Action을 가져와
    /*if (R[shostate][possibleAction] >= 0) { //<- R 값에서 +값이면 Q테이블에 적용 
        Q[shostate][possibleAction] = reward(shostate, possibleAction);
        
        //activateAction(possibleAction);//0보다 큰 값을 실행
        //delay(500);
    }*/
    Q[shostate][possibleAction] = reward(shostate, possibleAction);

}
int getRandomAction(int upperBound, int lowerBound, int currentState) {//최대 - 최소//upperCound == qsize == 9, lowerBound == 0 // 1초당 진행 (1회씩 진행)
    int action;
    int choiceIsValid = 0;
    //Randomly choose a possible action connected to the current state.
   // do {
        //Get a random value between 0 and 4.
        action = lowerBound + rand() % AAction;
        activateAction(action);//<-학습 시키는 쇼
        //delay(500);
        /*if (R[currentState][action] >= 0) {//<-R테이블에서 0 이상이면 break
            printf("stop");//<-bbark();
            break;
        }*/
    //} while (choiceIsValid == 0);
    return action;
}

float reward(int RcurrentState, int action) {//축적된 R과, maximum에서의 값 == R + Q(g*maximum)
    return R[RcurrentState][action] + Q[RcurrentState][action] * 0.8;
}
/*
float reward(int action) {//축적된 R과, maximum에서의 값 == R + Q(g*maximum)
    return R[currentState][action] + (mygamma * maximum(action, 0));
}
*/
int main() {
    setup();
    //int primeState;
    //primeState = nowstates();//<-아두이노에서 보자고
    //printf("%d",primeState);
    int currentState;

    for (int i = 0; i < 10; i++) {//state마다 작동
        currentState = i;
        int newState = 0;
        newState = maximum(currentState, 1);//최선의 값
        printf("%d", currentState);
        currentState = newState;
        activateAction(currentState); // <- 동작
        //<-딜레이
        printf("\n");
    } // i  

}

int nowstates() {
    int Fobstacle, Zobs, numstate;
    
    Fobstacle = obscheck(0);
    Zobs = obscheck(1);
    
    int pm;
    pm = rand()%2;
    if (pm == 1)//Z값 무작위 +,- 부여
    {
        Zobs = -Zobs;
    }

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

int obscheck(int need) {//<-아두이노에서 보자고
    int Fobs = rand() % 40, Lobs = rand() % 40, Robs = rand() % 40, Z = 0;
    /*  Fobs = checkUlt(90);//in servo pos;
      Robs = checkUlt(60);//in servo pos;
      Lobs = checkUlt(120);//in servo pos;
      Z = Lobs - Robs;
    *//*
      scanf_s("Fobs : %d", &Fobs,sizeof(Fobs));
      scanf_s("Lobs : %d", &Lobs, sizeof(Fobs));
      scanf_s("Robs : %d", &Robs, sizeof(Fobs));*/
    Z = Lobs - Robs;

    if (need == 0) {
        return Fobs;
    }
    else {
        return Z;
    }
}

void activateAction(int Actnum) { //Action 0. 좌회전, 1. 우회전, 2. 전진, 3. 후진
    if (Actnum == 0) {
        //leftturn();
        printf("leftturn");
    }
    else if (Actnum == 1) {
        //rightturn();
        printf("righttrun");
    }
    else if (Actnum == 2) {
        //forward();
        printf("forward");
    }
    else {
        //back();
        printf("back");
    }
}

int maximum(int state, int returnIndexOnly) {
    // if returnIndexOnly = 1, a Q matrix index is returned.
    // if returnIndexOnly = 0, a Q matrix element is returned.
    int winner;
    int foundNewWinner;
    int done = 0;
    winner = 0;
    do {
        foundNewWinner = 0;
        for (int i = 0; i < 4; i++) {//state에 대한 최고값 추출
            if ((i < winner) || (i > winner)) {//Avoid self-comparison. // i != winner일 때
                if (Q[state][i] > Q[state][winner]) {
                    winner = i;
                    foundNewWinner = 1; // 최종값을 찾았다.
                }
            }
        } // i

        if (foundNewWinner == 0) {//못찾았다면 break
            done = 1; // break
        }
    } while (done == 0);

    if (returnIndexOnly == 1) {//1인 경우 winner 리턴, 0인경우 R테이블에 대해 reward값을 더한다.
        return winner;
    }
    else {
        return Q[state][winner];
    }
}

int stoplearn() {
    int stop=0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 4; j++) {
            if (Q[i][j] > 40)
            {
                stop = 1;
                return stop;
            }
        } // j
    } // i
}