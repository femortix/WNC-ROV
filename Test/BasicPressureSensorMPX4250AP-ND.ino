int PP = 0; // pressure pin number analog pin 0
int Pvalue = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PP,INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  Pvalue = analogRead(PP);
  Serial.println(Pvalue);
  delay(100);

}
