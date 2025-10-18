volatile long int count = 0; 
int pwmPin = 5;  // PWM output for motor control
int dirPin = 6;  // Direction control
double wheelRadius = 2.15; // Adjust for accurate distance calculation
double distanceToMaxSpeed = 5; // Halfway point in feet
double totalDistance = 10; // Total travel distance in feet
double maxSpeed = 16; // Target max speed in m/s

double acceleration; // Calculated acceleration
double velocity = 0; // Current velocity
double position = 0; // Current position in meters
bool returning = false; // Flag for return motion
unsigned long prevTime = 0; // Time tracking for acceleration control

void setup() {
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    attachInterrupt(digitalPinToInterrupt(2), encoderISR, RISING); // Encoder Interrupt
    Serial.begin(9600);

    pinMode(pwmPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    digitalWrite(dirPin, HIGH); // Set motor direction forward
    
    acceleration = (maxSpeed * maxSpeed) / (2 * distanceToMaxSpeed * 0.3048); // Convert ft to meters
}

void loop() {
    unsigned long currentTime = millis();
    double deltaTime = (currentTime - prevTime) / 1000.0; // Convert ms to seconds
    prevTime = currentTime;

    if (!returning) {
        // Moving forward
        if (position < totalDistance * 0.3048) { // Convert ft to meters
            if (position < distanceToMaxSpeed * 0.3048) {
                velocity += acceleration * deltaTime; // Accelerate
                if (velocity > maxSpeed) velocity = maxSpeed; // Limit max speed
            } else {
                velocity = maxSpeed; // Maintain max speed
            }
            position += velocity * deltaTime; // Update position
        } else {
            // Reverse direction once we reach 10 ft
            returning = true;
            digitalWrite(dirPin, HIGH); // Reverse motor direction
            velocity = 16; // Reset velocity for smooth deceleration
        }
    } else {
        // Moving back
        if (position > 0) {
            if (position > distanceToMaxSpeed * 0.3048) {
                velocity += acceleration * deltaTime; // Accelerate back
                if (velocity > maxSpeed) velocity = maxSpeed;
            } else {
                velocity = maxSpeed; // Maintain speed
            }
            position -= velocity * deltaTime; // Decrease position
        } else {
            analogWrite(pwmPin, 0); // Stop the motor at start
            Serial.println("Back at starting point!");
            while(1); // Halt loop
        }
    }

    int pwmValue = map(velocity * 100, 0, maxSpeed * 100, 0, 255); // Convert speed to PWM
    analogWrite(pwmPin, pwmValue);

    Serial.print("Direction: ");
    Serial.print(returning ? "Returning " : "Forward ");
    Serial.print("| Velocity: ");
    Serial.print(velocity);
    Serial.print(" m/s | Position: ");
    Serial.print(position);
    Serial.println(" m");
}

// Interrupt Service Routine (ISR) for Encoder Counting
void encoderISR() {
    if (digitalRead(3) == HIGH)
        count++;
    else
        count--;
}
