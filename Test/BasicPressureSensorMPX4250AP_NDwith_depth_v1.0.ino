int PP = 0; // pressure pin number analog pin 0
int Pvalue = 0;
float Ppercent = 0;
float Pkpa = 0;
float depth = 0;
float surfaceP = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PP,INPUT);
  surfaceP = 250*(float(analogRead(PP))/1023)+25;
  Serial.println(surfaceP);
  delay(1000);

}

void loop() {
  // put your main code here, to run repeatedly:
  Pvalue = analogRead(PP);
  Serial.println(Pvalue);
  Ppercent = float(Pvalue)/1023;
  Serial.println(Ppercent);
  Pkpa = 250*Ppercent+25;
  Serial.println(Pkpa);
  depth = (Pkpa - surfaceP)/9.81;
  Serial.print("depth = ");
  Serial.println(depth);
  delay(100);

}
