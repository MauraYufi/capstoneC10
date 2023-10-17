#include <Fuzzy.h>

Fuzzy *fuzzy = new Fuzzy();

// Fuzzy input gelombang
FuzzySet *low = new FuzzySet(0, 0, 0.5 , 2.5);
FuzzySet *medium = new FuzzySet(0.5, 2.5, 2.5, 4);
FuzzySet *high = new FuzzySet(2.5, 4, 14, 14);
  
// Fuzzy input angin
FuzzySet *slow = new FuzzySet(0, 0, 12, 20);
FuzzySet *moderate = new FuzzySet(12, 20, 28, 38);
FuzzySet *strong = new FuzzySet(28, 38, 64, 64);
  
  // Fuzzy output angin
FuzzySet *safe = new FuzzySet(0, 0, 0, 0.5);
FuzzySet *normal = new FuzzySet(0, 0.5, 0.5, 1);
FuzzySet *danger = new FuzzySet(0.5, 1, 1, 1);

void setup()
{
  // Set the Serial output
  Serial.begin(9600);

  // Instantiating a Fuzzy object
  
  
  

 
  //fuzzyinput
  FuzzyInput *wave = new FuzzyInput(1);
  wave->addFuzzySet(low);
  wave->addFuzzySet(medium);
  wave->addFuzzySet(high);
  fuzzy->addFuzzyInput(wave);

  //fuzzyinput
  FuzzyInput *wind = new FuzzyInput(2);
  wind->addFuzzySet(slow);
  wind->addFuzzySet(moderate);
  wind->addFuzzySet(strong);
  fuzzy->addFuzzyInput(wind);

  //fuzzyotput
  FuzzyOutput *rate = new FuzzyOutput(1);
  rate->addFuzzySet(safe);
  rate->addFuzzySet(normal);
  rate->addFuzzySet(danger);
  fuzzy->addFuzzyOutput(rate);

  //1
  FuzzyRuleAntecedent *low_slow = new FuzzyRuleAntecedent();
  low_slow->joinWithOR(low,slow);

  FuzzyRuleConsequent *rate_safe = new FuzzyRuleConsequent();
  rate_safe->addOutput(safe);

  FuzzyRule *fuzzyRule01 = new FuzzyRule(1, low_slow, rate_safe);
  fuzzy->addFuzzyRule(fuzzyRule01);

  //2
  FuzzyRuleAntecedent *medium_moderate = new FuzzyRuleAntecedent();
  medium_moderate->joinWithAND(medium,moderate);

  FuzzyRuleConsequent *rate_normal = new FuzzyRuleConsequent();
  rate_normal->addOutput(normal);

  FuzzyRule *fuzzyRule02 = new FuzzyRule(2, medium_moderate, rate_normal);
  fuzzy->addFuzzyRule(fuzzyRule02);

  //3
  FuzzyRuleAntecedent *high_strong = new FuzzyRuleAntecedent();
  high_strong->joinWithOR(high,strong);

  FuzzyRuleConsequent *rate_danger = new FuzzyRuleConsequent();
  rate_danger->addOutput(danger);

  FuzzyRule *fuzzyRule03 = new FuzzyRule(3, high_strong, rate_danger);
  fuzzy->addFuzzyRule(fuzzyRule03);
}

void loop()
{
  float in_wave = 2000;
  float in_wind = 0;

  Serial.print("\t\t\tWave: ");
  Serial.println(in_wave);
  Serial.print("\t\t\tWind: ");
  Serial.println(in_wind);

  fuzzy->setInput(1, in_wave);
  fuzzy->setInput(2, in_wind);
  fuzzy->fuzzify();

  float output = fuzzy->defuzzify(1);

  Serial.println("Result: ");
  Serial.print("\t\t\tRate: ");
  Serial.println(output);

  // wait 12 seconds
  delay(12000);
}
