const int B_1A = 7;
const int B_1B = 6;

const int A_1A = 5;
const int A_1B = 4;

const int MOTOR_SPEED = 250;  // 0～255

void stopAll(){
  // analogWrite()したピンも確実に停止させる
  analogWrite(B_1A, 0);
  analogWrite(B_1B, 0);
  analogWrite(A_1A, 0);
  analogWrite(A_1B, 0);
}

void setup(){
  pinMode(B_1A, OUTPUT);
  pinMode(B_1B, OUTPUT);
  pinMode(A_1A, OUTPUT);
  pinMode(A_1B, OUTPUT);

  stopAll();
}

//loop部分
#if 1
void loop(){
  // =========================
  // A・B 同時に正転
  // =========================
  analogWrite(B_1A, MOTOR_SPEED);
  analogWrite(B_1B, 0);
  analogWrite(A_1A, MOTOR_SPEED);
  analogWrite(A_1B, 0);
  delay(1000);

  // =========================
  // A・B 同時に停止
  // =========================
  stopAll();
  delay(1000);

  // =========================
  // A・B 同時に反転
  // =========================
  analogWrite(B_1A, 0);
  analogWrite(B_1B, MOTOR_SPEED);
  analogWrite(A_1A, 0);
  analogWrite(A_1B, MOTOR_SPEED);
  delay(1000);

  // =========================
  // A・B 同時に停止
  // =========================
  stopAll();
  delay(1000);
}

#else //正転、反転確認


void loop(){
  // B 正転
  analogWrite(B_1A, MOTOR_SPEED);
  digitalWrite(B_1B, LOW);
  delay(1000);

  stopAll();
  delay(300);

  // B 反転
  digitalWrite(B_1A, LOW);
  analogWrite(B_1B, MOTOR_SPEED);
  delay(1000);

  stopAll();
  delay(300);

  // A 正転
  analogWrite(A_1A, MOTOR_SPEED);
  digitalWrite(A_1B, LOW);
  delay(1000);

  stopAll();
  delay(300);

  // A 反転
  digitalWrite(A_1A, LOW);
  analogWrite(A_1B, MOTOR_SPEED);
  delay(1000);

  stopAll();
  delay(1000);
}


#endif
