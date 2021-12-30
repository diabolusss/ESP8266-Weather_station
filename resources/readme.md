place for custom resources like images, pdf, etc.

# some useful links
http://paulmurraycbr.github.io/ArduinoTheOOWay.html
https://github.com/maarten-pennings/CCS811/issues/8
https://github.com/JsBergbau/CCS811-Firmware-Updater

@see Ticker example
class ExampleClass {
  public:
    ExampleClass(int pin, int duration) : _pin(pin), _duration(duration) {
      pinMode(_pin, OUTPUT);
      _myTicker.attach_ms(_duration, std::bind(&ExampleClass::classBlink, this));  //possible way to attach ISR and rewrite button lib into non static way
    }
    ~ExampleClass() {};

    int _pin, _duration;
    Ticker _myTicker;

    void classBlink() {
      digitalWrite(_pin, !digitalRead(_pin));
    }
};
