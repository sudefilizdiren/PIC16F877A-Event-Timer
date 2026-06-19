/* ==============================================================================
 * PROJECT OVERVIEW: Event Timer
 * ==============================================================================
 * * Group Members:
 *   Sude Filiz Diren 64230144 CoE
 *   Emre Ekici       64230150 CoE
 * 
 * * Description:
 * This project is a versatile countdown timer built for a PIC microcontroller 
 * (PIC16F877A) running at 20 MHz. It uses a 4x4 matrix keypad for 
 * user input, a 16x2 character LCD to display the interface, and a PWM output 
 * to drive a passive buzzer for an alert melody.
 * 
 * * Key Features & Flow:
 * 1. Setup Phase: The user inputs the desired countdown time in seconds using 
 * the keypad (up to 9999 seconds).
 * 2. Start/Clear: Pressing the '#' key starts the countdown. Pressing the '*' 
 * key clears the current input.
 * 3. Countdown: Once started, Timer1 generates an interrupt every 100ms. Ten 
 * interrupts make up 1 second, decrementing the timer. The remaining time 
 * is dynamically updated on the LCD in MM:SS format.
 * 4. Time's Up: When the countdown reaches 0, the timer stops, and the hardware 
 * PWM module plays the "Pirates of the Caribbean" melody. This loops until 
 * the user acknowledges it by pressing the '*' key.
 * 5. Emergency Stop: A dedicated hardware button connected to the External 
 * Interrupt pin (RB0) acts as an emergency stop. If triggered at any time, 
 * the timer immediately halts, the LCD flashes an emergency message, and a 
 * high-low two-tone siren plays continuously until reset with the '*' key.
 * * Hardware Connections:
 * - PORTB & PORTC: 4x4 Keypad (Rows and Cols)
 * - PORTD: 16x2 LCD (4-bit mode)
 * - PORTB.RB0: Emergency Stop Button (Active Low/Falling Edge Triggered)
 * - PORTC.RC2 (CCP1): Speaker/Buzzer for PWM audio generation
 * ==============================================================================
 */

#include <xc.h>
#include <stdio.h>

// --- CONFIGURATION BITS ---
#pragma config FOSC = HS        // High-Speed Oscillator (since we use a 20 MHz crystal)
#pragma config WDTE = OFF       // Watchdog Timer Disabled (prevents random resets)
#pragma config PWRTE = ON       // Power-up Timer Enabled (waits for power to stabilize)
#pragma config BOREN = ON       // Brown-out Reset Enabled (resets if voltage drops)
#pragma config LVP = OFF        // Low-Voltage Programming Disabled (frees up RB3/PGM pin)

#define _XTAL_FREQ 20000000     // Define system frequency as 20 MHz for __delay functions

// --- LCD PIN DEFINITIONS (PORTD) ---
// Using 4-bit mode to save microcontroller pins
#define RS PORTDbits.RD2        // Register Select (0 = Command, 1 = Data)
#define EN PORTDbits.RD3        // Enable pin (latches data on falling edge)
#define D4 PORTDbits.RD4        // Data bit 4
#define D5 PORTDbits.RD5        // Data bit 5
#define D6 PORTDbits.RD6        // Data bit 6
#define D7 PORTDbits.RD7        // Data bit 7

// --- GLOBAL VARIABLES ---
volatile unsigned int countdown_seconds = 0; // Stores the total seconds left
volatile bit is_running = 0;                 // Flag: 1 if timer is actively counting, 0 if paused/stopped
volatile bit emergency_flag = 0;             // Flag: 1 if external interrupt (RB0) was triggered
volatile unsigned char timer_tick_count = 0; // Counts 100ms interrupts (10 ticks = 1 second)

// --- FUNCTION PROTOTYPES ---
void Peripheral_Init(void);
void LCD_Init(void);
void LCD_Cmd(unsigned char cmd);
void LCD_Char(unsigned char data);
void LCD_String(const char *str);
void LCD_Clear(void);
char Keypad_Read(void);
void PWM_Set_Freq(unsigned int freq);
char Delay_With_KeyCheck(unsigned int ms);
void Play_Melody_TimeUp(void);
void Play_Siren_Emergency(void);

// --- INTERRUPT SERVICE ROUTINE (ISR) ---
void __interrupt() ISR(void) {
    // 1. External Interrupt (RB0 pin triggered)
    if (INTCONbits.INTF) { 
        emergency_flag = 1;       // Signal main loop that an emergency occurred
        is_running = 0;           // Stop the countdown
        countdown_seconds = 0;    // Reset the time
        T1CONbits.TMR1ON = 0;     // Turn off Timer1
        INTCONbits.INTF = 0;      // Clear the External Interrupt flag so it doesn't loop forever
    }
    
    // 2. Timer1 Interrupt - Triggered exactly every 100ms
    if (PIR1bits.TMR1IF) {
        /* How the math works for 100ms at 20MHz:
         * Instruction clock = 20MHz / 4 = 5MHz (0.2us per tick)
         * Timer1 Prescaler = 1:8 -> Timer clock = 5MHz / 8 = 625kHz (1.6us per tick)
         * To get 100ms: 100,000us / 1.6us = 62500 ticks needed.
         * Timer1 counts up to 65536. So we preload it with (65536 - 62500) = 3036.
         * 3036 in hexadecimal is 0x0BDC.
         */
        TMR1H = 0x0B;         // Load high byte of 3036
        TMR1L = 0xDC;         // Load low byte of 3036
        
        if (is_running) {
            timer_tick_count++;
            // 10 ticks of 100ms = 1 full second
            if (timer_tick_count >= 10) { 
                timer_tick_count = 0;
                if (countdown_seconds > 0) {
                    countdown_seconds--; // Decrement the actual timer
                }
            }
        }
        PIR1bits.TMR1IF = 0;  // Clear Timer1 Interrupt flag
    }
}

// --- MAIN ROUTINE ---
void main(void) {
    char lcd_buffer[16];
    char key;
    unsigned int minutes, seconds;

    Peripheral_Init();  // Setup pins, timers, and interrupts
    LCD_Init();         // Initialize the screen
    
    LCD_String(" Timer Setup  ");
    __delay_ms(1000);   // Show welcome screen for 1 second
    LCD_Clear();

    while(1) {
        // --- EMERGENCY STATE ---
        if (emergency_flag) {
            LCD_Clear();
            LCD_String(" EMERGENCY STOP ");
            LCD_Cmd(0xC0); // 0xC0 sets cursor to the beginning of the second line
            LCD_String("Press * to Reset");
            
            // Play siren (Loops internally until the '*' key is pressed)
            Play_Siren_Emergency(); 
            
            emergency_flag = 0;  // Clear the flag after user resets
            PWM_Set_Freq(0);     // Mute the speaker
            LCD_Clear();
        }
        // --- TIME'S UP STATE ---
        else if (is_running && countdown_seconds == 0) {
            is_running = 0;
            T1CONbits.TMR1ON = 0; // Turn off Timer1 to save power/cycles
            
            LCD_Clear();
            LCD_String("  TIME IS UP!  ");
            LCD_Cmd(0xC0); // Move to second line
            LCD_String("Press * to Reset");
            
            // Play Pirates of the Caribbean (Long Version)
            Play_Melody_TimeUp();
            
            PWM_Set_Freq(0); // Mute the speaker after reset
            LCD_Clear();
        }
        // --- COUNTDOWN RUNNING STATE ---
        else if (is_running) {
            minutes = countdown_seconds / 60;
            seconds = countdown_seconds % 60;
            
            LCD_Cmd(0x80); // 0x80 sets cursor to the beginning of the first line
            LCD_String(" Counting Down  ");
            LCD_Cmd(0xC0); // 0xC0 sets cursor to the beginning of the second line
            
            // Format time as MM:SS with leading zeros
            sprintf(lcd_buffer, "    %02u:%02u      ", minutes, seconds);
            LCD_String(lcd_buffer);
            
            __delay_ms(100); // Small delay to prevent LCD flickering
        }
        // --- SETUP/IDLE STATE ---
        else {
            LCD_Cmd(0x80); // Move to first line
            LCD_String("Set Secs Press #");
            
            LCD_Cmd(0xC0); // Move to second line
            // Display currently entered seconds (max 9999)
            sprintf(lcd_buffer, "Time: %04u sec ", countdown_seconds);
            LCD_String(lcd_buffer);
            
            key = Keypad_Read();
            if (key != 0) {
                // If the user pressed a number (0-9)
                if (key >= '0' && key <= '9') {
                    // Shift numbers left (e.g., typing '1' then '2' becomes 12)
                    countdown_seconds = (countdown_seconds * 10) + (key - '0');
                    if (countdown_seconds > 9999) countdown_seconds = 9999; // Cap at 9999
                } 
                // If the user pressed '#' to START
                else if (key == '#') { 
                    if (countdown_seconds > 0) {
                        LCD_Clear();
                        timer_tick_count = 0;
                        is_running = 1;
                        T1CONbits.TMR1ON = 1; // Turn ON Timer1 module
                    }
                } 
                // If the user pressed '*' to CLEAR
                else if (key == '*') { 
                    countdown_seconds = 0;
                }
                
                // Wait until the user lifts their finger off the key to avoid reading multiple times
                while(Keypad_Read() != 0); 
                __delay_ms(50); // Debounce delay
            }
        }
    }
}

// --- PERIPHERAL INITIALIZATION ---
void Peripheral_Init(void) {
    // 1. I/O Port Configuration
    // 0x1F is 0001 1111 in binary. RB0-RB4 are inputs (buttons/ext int), RB5-RB7 are outputs (keypad rows).
    TRISB = 0x1F;               
    OPTION_REGbits.nRBPU = 0;   // Enable internal pull-up resistors for PORTB (needed for keypad)
    
    TRISCbits.TRISC0 = 0;       // Set RC0 as an output (used for the 4th row of the keypad)
    PORTCbits.RC0 = 1;          // Set it high initially
    
    TRISCbits.TRISC2 = 0;       // Set RC2 (CCP1 pin) as an output for the PWM speaker
    
    TRISD = 0x00;               // 0x00 sets all PORTD pins as outputs (for the LCD)
    
    // 2. External Interrupt Configuration (RB0)
    OPTION_REGbits.INTEDG = 0;  // 0 = Interrupt on falling edge (triggers when button connects to ground)
    INTCONbits.INTF = 0;        // Clear any pending interrupt flags
    
    // 3. Timer1 Configuration (16-bit timer for 100ms ticks)
    // 0x30 is 0011 0000. Bits 5-4 are 11 (1:8 Prescaler). Bit 0 is 0 (Timer initially OFF).
    T1CON = 0x30;               
    TMR1H = 0x0B;               // Preload 3036 (0x0BDC) for 100ms delay at 20MHz
    TMR1L = 0xDC;               
    PIR1bits.TMR1IF = 0;        // Clear Timer1 interrupt flag
    
    // 4. PWM Configuration for Audio generation (Using CCP1 and Timer2)
    // 0x0C is 0000 1100. Lower nibble 1100 configures CCP1 module for PWM mode.
    CCP1CON = 0x0C;             
    PR2 = 0xFF;                 // Set PWM period to maximum initially (0xFF = 255)
    CCPR1L = 0x00;              // 0 Duty cycle -> Speaker is Muted initially
    // 0x06 is 0000 0110. Bit 2 turns Timer2 ON. Bits 1-0 set Prescaler to 1:16.
    T2CON = 0x06;               
    
    // 5. Global Interrupt Enables
    INTCONbits.INTE = 1;        // Enable RB0 External Interrupt
    PIE1bits.TMR1IE = 1;        // Enable Timer1 Overflow Interrupt
    INTCONbits.PEIE = 1;        // Enable Peripheral Interrupts (required for Timer1)
    INTCONbits.GIE = 1;         // Enable Global Interrupts (turns on the master switch)
}

// --- KEYPAD READ FUNCTION ---
// Scans the 4x4 matrix keypad by pulling rows low one by one and reading the columns.
char Keypad_Read(void) {
    PORTBbits.RB5 = 1; PORTBbits.RB6 = 1; PORTBbits.RB7 = 1; PORTCbits.RC0 = 1;
    
    // ROW 1
    PORTBbits.RB5 = 0; __delay_us(100);
    if(!PORTBbits.RB1) { PORTBbits.RB5 = 1; return '1'; }
    if(!PORTBbits.RB2) { PORTBbits.RB5 = 1; return '2'; }
    if(!PORTBbits.RB3) { PORTBbits.RB5 = 1; return '3'; }
    if(!PORTBbits.RB4) { PORTBbits.RB5 = 1; return 'A'; }
    PORTBbits.RB5 = 1;
    
    // ROW 2
    PORTBbits.RB6 = 0; __delay_us(100);
    if(!PORTBbits.RB1) { PORTBbits.RB6 = 1; return '4'; }
    if(!PORTBbits.RB2) { PORTBbits.RB6 = 1; return '5'; }
    if(!PORTBbits.RB3) { PORTBbits.RB6 = 1; return '6'; }
    if(!PORTBbits.RB4) { PORTBbits.RB6 = 1; return 'B'; }
    PORTBbits.RB6 = 1;
    
    // ROW 3
    PORTBbits.RB7 = 0; __delay_us(100);
    if(!PORTBbits.RB1) { PORTBbits.RB7 = 1; return '7'; }
    if(!PORTBbits.RB2) { PORTBbits.RB7 = 1; return '8'; }
    if(!PORTBbits.RB3) { PORTBbits.RB7 = 1; return '9'; }
    if(!PORTBbits.RB4) { PORTBbits.RB7 = 1; return 'C'; }
    PORTBbits.RB7 = 1;
    
    // ROW 4
    PORTCbits.RC0 = 0; __delay_us(100);
    if(!PORTBbits.RB1) { PORTCbits.RC0 = 1; return '*'; }
    if(!PORTBbits.RB2) { PORTCbits.RC0 = 1; return '0'; }
    if(!PORTBbits.RB3) { PORTCbits.RC0 = 1; return '#'; }
    if(!PORTBbits.RB4) { PORTCbits.RC0 = 1; return 'D'; }
    PORTCbits.RC0 = 1;
    
    return 0; // Return 0 if no button is pressed
}

// --- LCD DRIVER FUNCTIONS ---
void LCD_Pulse_Enable(void) {
    EN = 1; __delay_us(50); EN = 0; __delay_us(50); // Pulse the Enable pin to latch data into LCD
}

void LCD_Cmd(unsigned char cmd) {
    RS = 0; // RS = 0 means we are sending a Command (like clear, move cursor)
    // Send the high 4 bits (nibble) first
    D4 = (cmd >> 4) & 1; D5 = (cmd >> 5) & 1; D6 = (cmd >> 6) & 1; D7 = (cmd >> 7) & 1;
    LCD_Pulse_Enable();
    // Send the low 4 bits (nibble) next
    D4 = cmd & 1; D5 = (cmd >> 1) & 1; D6 = (cmd >> 2) & 1; D7 = (cmd >> 3) & 1;
    LCD_Pulse_Enable();
    __delay_ms(2); // Some commands take time to execute
}

void LCD_Char(unsigned char data) {
    RS = 1; // RS = 1 means we are sending Data (ASCII characters to display)
    // Send the high 4 bits
    D4 = (data >> 4) & 1; D5 = (data >> 5) & 1; D6 = (data >> 6) & 1; D7 = (data >> 7) & 1;
    LCD_Pulse_Enable();
    // Send the low 4 bits
    D4 = data & 1; D5 = (data >> 1) & 1; D6 = (data >> 2) & 1; D7 = (data >> 3) & 1;
    LCD_Pulse_Enable();
    __delay_us(50); // Character writing is fast
}

void LCD_Init(void) {
    __delay_ms(20); // Wait for LCD power to stabilize
    RS = 0; EN = 0;
    
    // The specific startup sequence required for HD44780 LCD controllers to enter 4-bit mode
    D4=1; D5=1; D6=0; D7=0; LCD_Pulse_Enable(); __delay_ms(5);
    D4=1; D5=1; D6=0; D7=0; LCD_Pulse_Enable(); __delay_us(150);
    D4=1; D5=1; D6=0; D7=0; LCD_Pulse_Enable();
    D4=0; D5=1; D6=0; D7=0; LCD_Pulse_Enable(); // Tell it to use 4-bit mode
    
    // 0x28 = 2 Lines, 5x8 Font, 4-bit mode
    LCD_Cmd(0x28); 
    // 0x0C = Display ON, Cursor OFF, Blinking OFF
    LCD_Cmd(0x0C); 
    // 0x06 = Entry mode: Auto-increment cursor position to the right
    LCD_Cmd(0x06); 
    // 0x01 = Clear the entire display
    LCD_Cmd(0x01); 
}

void LCD_String(const char *str) {
    // Loop through the character array until the null terminator '\0' is found
    while(*str != '\0') {
        LCD_Char(*str++);
    }
}

void LCD_Clear(void) {
    LCD_Cmd(0x01); // 0x01 is the standard LCD command to clear the screen
}

// --- MELODY & PWM FUNCTIONS ---

void PWM_Set_Freq(unsigned int freq) {
    // Hardware constraint: At 20MHz with 1:16 prescaler, frequencies below ~1220Hz 
    // cause the PR2 register calculation to exceed 255 (which is the 8-bit limit).
    if (freq < 1220) { 
        CCPR1L = 0; // If frequency is 0 or too low, just turn the duty cycle to 0 (MUTE)
        return;
    }
    
    // Formula to calculate PWM Period Register (PR2):
    // PR2 = [Fosc / (4 * PWM_Freq * TMR2_Prescaler)] - 1
    // At 20MHz and 1:16 Prescaler: PR2 = [312500 / Freq] - 1
    unsigned long pr2_val = (312500 / freq) - 1;
    if (pr2_val > 255) pr2_val = 255; // Safety cap for 8-bit register
    
    PR2 = (unsigned char)pr2_val;
    
    // Duty cycle determines volume. Setting it to PR2/2 gives a 50% duty cycle (max square wave volume).
    CCPR1L = PR2 / 2; 
}

// Custom delay function that constantly checks if the user pressed the '*' key.
// Returns 1 if aborted, 0 if the delay finished normally.
char Delay_With_KeyCheck(unsigned int ms) {
    for(unsigned int i = 0; i < ms; i++) {
        if(Keypad_Read() == '*') return 1; // User cancelled the alarm
        __delay_ms(1);
    }
    return 0; // Not cancelled, time is up
}

void Play_Melody_TimeUp(void) {
    // 'const' is used so this array is saved in Program Memory (Flash) rather than Data RAM.
    // Microcontrollers have very little RAM, and a large array will cause an error without 'const'.
    const unsigned int notes[62] = {
        1760, 2093, 2349, 2349, 2349, 2637, 2794, 2794, 2794, 3136, 2637, 2637, 2349, 2093, 2093, 2349,
        1760, 2093, 2349, 2349, 2349, 2637, 2794, 2794, 2794, 3136, 2637, 2637, 2349, 2093, 2349,
        1760, 2093, 2349, 2349, 2349, 2794, 3136, 3136, 3136, 3520, 3729, 3729, 3520, 3136, 3520, 2349,
        2349, 2637, 2794, 2794, 3136, 3520, 2349, 2349, 2349, 2794, 2637, 2637, 2349, 2093, 2349
    };
    
    const unsigned int durations[62] = {
        100, 100, 200, 200, 100, 100, 200, 200, 100, 100, 200, 200, 100, 100, 100, 300,
        100, 100, 200, 200, 100, 100, 200, 200, 100, 100, 200, 200, 100, 100, 400,
        100, 100, 200, 200, 100, 100, 200, 200, 100, 100, 200, 200, 100, 100, 100, 300,
        100, 100, 200, 200, 100, 100, 200, 200, 100, 100, 200, 200, 100, 100, 400
    };

    while(1) { 
        for(int i = 0; i < 62; i++) {
            PWM_Set_Freq(notes[i]); // Generate the specific musical note frequency
            
            // Wait for the note's duration. If KeyCheck returns 1, the user pressed '*', so we exit.
            if(Delay_With_KeyCheck(durations[i])) return; 
            
            // Introduce a very short 30ms silence between notes to give a staccato (short, disconnected) rhythm feel
            PWM_Set_Freq(0); 
            if(Delay_With_KeyCheck(30)) return; 
        }
        
        // Wait 1 second in silence before repeating the melody from the beginning
        PWM_Set_Freq(0);
        if(Delay_With_KeyCheck(1000)) return; 
    }
}

void Play_Siren_Emergency(void) {
    while(1) { // Repeat the siren indefinitely
        PWM_Set_Freq(1500); // Low tone (1.5 kHz)
        if(Delay_With_KeyCheck(400)) return; // Play for 400ms, exit if '*' is pressed
        
        PWM_Set_Freq(2500); // High tone (2.5 kHz)
        if(Delay_With_KeyCheck(400)) return; // Play for 400ms, exit if '*' is pressed
    }
}
