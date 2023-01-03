#define F_CPU 100000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define N_ANIMATIONS 6
#define N_OPIONS 3
#define N_LEDS 16
#define ANIMATION_UPDATE_MAX 120
#define ANIMATION_UPDATE_STEP 12
#define BRIGHTNESS_UPDATE_STEP 20

volatile uint8_t* led_port[N_LEDS];
volatile uint8_t port_buffer[3] = {0, 0, 0};	
uint8_t led_mask[N_LEDS];

uint8_t button_pressed = 0;
uint8_t option = 0;

uint8_t animation = 0;
volatile uint8_t animation_step = 0;
uint8_t animation_update = ANIMATION_UPDATE_STEP;
uint8_t animation_t = 0;
const uint8_t animation_steps[N_ANIMATIONS] = {12, 2, 12, 6, 0, 20/*, 19*/};
	
void set(uint8_t pos);
void clear(uint8_t pos);
void toggle(uint8_t pos);

void anim0(void);
void anim1(void);
void anim2(void);
void anim3(void);
void anim4(void);
void anim5(void);
//void anim6(void);

void (*animations[N_ANIMATIONS])(void) = {anim0, anim1, anim2, anim3, anim4, anim5/*, anim6*/};

int main(void)
{	
	for (uint8_t i = 0; i < 6; ++i)
	{
		led_port[i] = &port_buffer[0];
		led_mask[i] = 1 << i;
	}
	for (uint8_t i = 6; i < 12; ++i)
	{
		led_port[i] = &port_buffer[1];
		led_mask[i] = 1 << (i - 6);
	}
	for (uint8_t i = 12; i < 16; ++i)
	{
		led_port[i] = &port_buffer[2];
    }
	led_mask[12] = 1 << 0;
	led_mask[13] = 1 << 1;
	led_mask[14] = 1 << 3;	
	led_mask[15] = 1 << 4;			
	
	DDRB |= 0x3F;
	DDRC |= 0x3F;
	DDRD |= 0x1B;
	
	PORTB &= ~0x3F;
	PORTC &= ~0x3F;
	PORTD &= ~0x1B;
	
	DDRD &= ~(1 << PORTD2);
	PORTD |= (1 << DDRD2);
	
	// F_CPU = 1 000 000 Hz
	// T = 16.384 ms
	TIFR |= (1 << TOV0);
	TIMSK |= (1 << TOIE0);
	TCCR0 = (1 << CS01) | (1 << CS00);
	
	// f = 1 000 000 Hz
	TIFR |= (1 << OCF2) | (1 << TOV2);
	TIMSK |= (1 << OCIE2) | (1 << TOIE2);
	OCR2 = 20;
	TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << CS20);
	
	sei();	
	
	while (1) 
    {

    }
}


ISR(TIMER0_OVF_vect)
{
	if ((PIND & (1 << PIND2)) == 0)
	{
		if (button_pressed < 255)
		{
			++button_pressed;
		}
	}
	else
	{
		// button pressed for long time
		if (button_pressed > 48)
		{
			++option;
			if (option == N_OPIONS)
			{
				option = 0;
			}
		}
		// button pressed for short time
		else if (button_pressed > 2)
		{
			if (option == 0)
			{
				++animation;
				if (animation == N_ANIMATIONS)
				{
					animation = 0;
				}
				animation_step = 0;
				port_buffer[0] &= 0xC0;
				port_buffer[1] &= 0xC0;
				port_buffer[2] &= 0xE4;
			}
			else if (option == 1)
			{
				animation_update += ANIMATION_UPDATE_STEP;
				if (animation_update == ANIMATION_UPDATE_MAX)
				{
					animation_update = ANIMATION_UPDATE_STEP;
				}
			}
			else if (option == 2)
			{
				uint8_t ocr = OCR2;
				if (255 - BRIGHTNESS_UPDATE_STEP < ocr)
				{
					ocr = BRIGHTNESS_UPDATE_STEP;
				}
				else
				{
					ocr += BRIGHTNESS_UPDATE_STEP;
				}
				OCR2 = ocr;
			}
		}
		button_pressed = 0;
	}
	
	++animation_t;
	if (animation_t >= animation_update)
	{
		animation_t = 0;
		(*animations[animation])();
		++animation_step;
		if (animation_step == animation_steps[animation])
		{
			animation_step = 0;
		}
	}
}

ISR(TIMER2_COMP_vect)
{
	PORTB &= 0xC0;
	PORTC &= 0xC0;
	PORTD &= 0xE4;
}

ISR(TIMER2_OVF_vect)
{
	PORTB |= port_buffer[0];
	PORTC |= port_buffer[1];
	PORTD |= port_buffer[2];
}

void set(uint8_t pos)
{
	*led_port[pos] |= led_mask[pos];
}

void clear(uint8_t pos)
{
	*led_port[pos] &= ~led_mask[pos];
}

void toggle(uint8_t pos)
{
	*led_port[pos] ^= led_mask[pos];
}

void anim0(void)
{
	if (animation_step >= 9)
	{
		return;
	}
	if (animation_step != 0 && animation_step != 8)
	{
		toggle(16 - animation_step);
	}
	toggle(animation_step);
}

void anim1(void)
{
	port_buffer[0] ^= 0x3F;
	port_buffer[1] ^= 0x3F;
	port_buffer[2] ^= 0x3B;
}

void anim2(void)
{
	if (animation_step >= 8)
	{
		return;
	}
	toggle(animation_step);
	toggle(animation_step + 8);
}

void anim3(void)
{
	if (animation_step == 5)
		return;
	if (animation_step == 0 || animation_step == 4)
	{
		toggle(animation_step);
		toggle(animation_step + 8);
	}
	else
	{
		toggle(animation_step);
		toggle(8 - animation_step);
		toggle(animation_step + 8);
		toggle(16 - animation_step);
	}
}

void anim4(void)
{
	port_buffer[0] |= 0x3F;
	port_buffer[1] |= 0x3F;
	port_buffer[2] |= 0x3B;
}

void anim5(void)
{
	if (animation_step >= 17)
	{
		return;
	}
	toggle(animation_step);
}

/*void anim6(void)
{
	if (animation_step >= 17)
	{
		port_buffer[0] &= 0xC0;
		port_buffer[1] &= 0xC0;
		port_buffer[2] &= 0xE4;
		return;
	}
	uint8_t t = animation_step << 1;
	if (animation_step > 8)
	{
		t -= 17;
	}
	set(t);
}*/
