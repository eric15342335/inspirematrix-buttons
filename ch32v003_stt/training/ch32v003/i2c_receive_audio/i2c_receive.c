#include "ch32v003fun.h"
#include "codebook.h"       // 16-frame-sample-blocks to numbers.
#include "dctm_20x8_8bit.h" // discrete-cosine transform matrix, 20mel->10cep
#include "i2c_events.h"
#include "mel_mx.h" // weights to generate 20 mel-scale bins from 65 FFT128 bins
#include "twiddles_RES13.h"
#include "music.h"
#include <stdio.h>

#define FFT 128
#define FF2 65
#define N 64   // 10ms frames
#define MEL 20 // numer of mel-scale bins to generate from FFT data
#define CEPS 8
#define WSIZE 25  // maximum word size in frames
#define MINSIZE 5 // minimum word size in frames

#define RES 8192 // resolution of FFT cos/sine to use...
#define SHIFT 13

#define TOL_ON 200
#define TOL_OFF 150

int16_t buffer[FFT];
int result, samcount = 0, total = 0, posn = 0, count = 0, lock, silcount = 0;

int re[FFT], im[FFT];
int mel[MEL];
u_int8_t nfloor[MEL];
int cep[CEPS];

int8_t word[CEPS * WSIZE], wsize;

//-----------------------------------------------------------------------------------
void init_timer() {
    // TIMER
    printf("Initializing timer...\r\n");
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;
    TIM1->CTLR1 |= TIM_CounterMode_Up | TIM_CKD_DIV1;
    TIM1->CTLR2 = TIM_MMS_1;
    TIM1->ATRLR = 80; // lower = higher sample rate. 800 for 6400sam/sec
    TIM1->PSC = 10 - 1;
    TIM1->RPTCR = 0;
    TIM1->SWEVGR = TIM_PSCReloadMode_Immediate;

    NVIC_EnableIRQ(TIM1_UP_IRQn);
    TIM1->INTFR = ~TIM_FLAG_Update;
    TIM1->DMAINTENR |= TIM_IT_Update;
    TIM1->CTLR1 |= TIM_CEN;
}

//-----------------------------------------------------------------------------------
// Handle the timer interrupt....
void TIM1_UP_IRQHandler(void) __attribute__((interrupt));
void TIM1_UP_IRQHandler() {
    if (TIM1->INTFR & TIM_FLAG_Update) {
        TIM1->INTFR = ~TIM_FLAG_Update;
        wait_for_event(I2C_EVENT_SLAVE_BYTE_RECEIVED);
        result = I2C1->DATAR;
        // Convert 8-bit unsigned to signed integer
        int16_t signed_sample = (int16_t)result - 127;
        //printf("Received sample: %d\n", signed_sample+127);
        //Delay_Ms(1000);
        // Store the sample in the buffer
        buffer[posn] = signed_sample;
        posn++;
        
        if (posn == FFT) {
            if (lock == 1) {
                printf("oops lock\n");
            } else {
                // Copy buffer to re[] and clear im[]
                for (int i = 0; i < FFT; i++) {
                    re[i] = (int)buffer[i];
                    im[i] = 0;
                }
                
                // Shift the buffer
                for (int i = 0; i < FFT - N; i++) {
                    buffer[i] = buffer[i + N];
                }
                
                posn = FFT - N;
                count++;
            }
        }
        /*result = result - 512;
        total += result;
        samcount++;
        if (samcount == 8) {
            total >>= 3;
            //total += 127;printf("%c", total);
            buffer[posn] = (int16_t)total;
            posn++;
            if (posn == FFT) {
                if (lock == 1)
                    printf("oops lock\n");
                for (posn = 0; posn < FFT; posn++) {
                    re[posn] = (int)buffer[posn];
                    im[posn] = 0;
                }

                for (posn = 0; posn < FFT - N; posn++)
                    buffer[posn] = buffer[posn + N];
                posn = FFT - N;
                count++;
            }
            samcount = 0;
            total = 0;
        }*/
    }
}

#define TxAdderss 0x03
#define RXAdderss 0x04

void IIC_Init(uint16_t address) {
    // Enable clocks for GPIOC and I2C1
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;
    RCC->APB1PCENR |= RCC_APB1Periph_I2C1;

    // Configure PC2 and PC1 as open-drain
    GPIOC->CFGLR &= ~GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 2);
    GPIOC->CFGLR |= GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 2);
    GPIOC->CFGLR &= ~GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 1);
    GPIOC->CFGLR |= GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 1);

    // Set module clock frequency
    uint32_t prerate =
        2000000; // I2C Logic clock rate, must be higher than the bus clock rate
    I2C1->CTLR2 |= (FUNCONF_SYSTEM_CORE_CLOCK / prerate) & I2C_CTLR2_FREQ;
    // 24MHz?
    // Set clock configuration
    uint32_t clockrate =
        1000000; // I2C Bus clock rate, must be lower than the logic clock rate
    I2C1->CKCFGR = ((FUNCONF_SYSTEM_CORE_CLOCK / (3 * clockrate)) & I2C_CKCFGR_CCR) |
                   I2C_CKCFGR_FS; // Fast mode 33% duty cycle
    // I2C1->CKCFGR = ((FUNCONF_SYSTEM_CORE_CLOCK/(25*clockrate))&I2C_CKCFGR_CCR) |
    // I2C_CKCFGR_DUTY | I2C_CKCFGR_FS; // Fast mode 36% duty cycle I2C1->CKCFGR =
    // (FUNCONF_SYSTEM_CORE_CLOCK/(2*clockrate))&I2C_CKCFGR_CCR; // Standard mode good to
    // 100kHz

    I2C1->OADDR1 = address; // Set own address
    I2C1->OADDR2 = 0;
    /* DO NOT SWAP THESE TWO ORDER !!!!!!!!!!!!!
    This bit is set and cleared by
    software and cleared by hardware when start is
    sent or PE=0.*/
    I2C1->CTLR1 |= I2C_CTLR1_PE;  // Enable I2C
    I2C1->CTLR1 |= I2C_CTLR1_ACK; // Enable ACK
    printf("\nInitializing I2C...\n");
        printf("Waiting for receiver address match!\n");
        wait_for_event(I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED);
        printf("I2C Event slave receiver address matched!\n");
        (void)I2C1->STAR2; // Clear ADDR flag
}

//-----------------------------------------------------------------------
void simple_int_fft(int size) {
    unsigned int even, odd, span, log = 0, rootindex; // indexes
    int temp;
    log = 0;

    for (span = size >> 1; span; span >>= 1, log++) {

        for (odd = span; odd < size; odd++) // iterate over the dual nodes
        {

            odd |= span;       // iterate over odd blocks only
            even = odd ^ span; // even part of the dual node pair
            // printf("even=%d,odd=%d\n",even,odd);

            temp = re[even] + re[odd];
            re[odd] = re[even] - re[odd];
            re[even] = temp;

            temp = im[even] + im[odd];
            im[odd] = im[even] - im[odd];
            im[even] = temp;

            rootindex = (even << log) & (size - 1); // find root of unity index
            if (rootindex)                          // skip rootindex[0] (has an identity)
            {
                // printf("rootindex=%d\n",rootindex);
                temp = re[odd] * tr[rootindex] / RES + im[odd] * ti[rootindex] / RES;
                im[odd] = im[odd] * tr[rootindex] / RES - re[odd] * ti[rootindex] / RES;

                // temp=((re[odd]*tr[rootindex])>>SHIFT)
                // +((im[odd]*ti[rootindex])>>SHIFT);
                // im[odd]=((im[odd]*tr[rootindex])>>SHIFT)
                // -((re[odd]*ti[rootindex])>>SHIFT);
                re[odd] = temp;
            }

        } // end of loop over n

    } // end of loop over FFT stages

} // end of function

//-----------------------------------------------------------------------
// returns log2 of input, multiplied by 8.....
const static int first[9] = {0, 0, 8, 12, 16, 18, 20, 22, 24};

unsigned int intlog2_8bit(unsigned int a) {
    unsigned int b;
    int r; // highest possible log result....
    int l; // log result
    int result;

    if (a > 8) {
        b = 0xffff0000;
        if ((a & b) == 0)
            r = 16;
        else
            r = 0;
        b = 0xff000000;
        b >>= r;
        if ((a & b) == 0)
            r += 8;
        b = 0xf0000000;
        b >>= r;
        if ((a & b) == 0)
            r += 4;
        b = 0xc0000000;
        b >>= r;
        if ((a & b) == 0)
            r += 2;
        b = 0x80000000;
        b >>= r;
        if ((a & b) == 0)
            r += 1;
        // printf("log2 a is %i\n",31-r);
        l = 31 - r;
        result = l * 8;
        a >>= (l - 3);
        result += a - 8;
        return (result);
    }
    else
        return (first[a]);
}

//-----------------------------------------------------------------------------------
int main() {
    SystemInit();

    IIC_Init(RXAdderss);
    //I2C1->CTLR1 &= I2C1->CTLR1;
    init_timer();
    int e, k = 10, i, n, lastcount = 0;
    int d, dist, bestdist, best, c;

    for (i = 0; i < MEL; i++)
        nfloor[i] = 192; // high initial noise floor

    while (1) {
        while (count != 0 && lastcount == count)
            k++; // wait till 10ms of samples read....
        if (count - lastcount != 1)
            printf("oops framecount\n");
        lock = 1;
        // lock=0;  // ignore locking for now when doing trace stuff.

        for (i = FFT - 1, n = FFT - 2; i != 0; i--, n--)
            re[i] = re[i] - re[n];
        re[0] = 0; // 1.0 pre-emph?
        for (i = 0; i < FFT; i++)
            re[i] <<= 8; // scale for the FFT
        simple_int_fft(FFT);
        for (i = 1; i < FFT; i += 2) {
            re[i] = (re[i] * re[i] + im[i] * im[i]);
            re[i] >>= 8;
        }
        // for (i=1; i<FFT; i+=2) if (re[i]>(1<<23)) printf("oops!\n");

        for (i = 0; i < MEL; i++)
            mel[i] = 0;
        i = 0;
        n = 0;
        while (n != MEL) {
            if (mel_mx[i] == 0) {
                i++;
                n++;
            }
            else {
                mel[n] += (unsigned int)re[mel_mx[i]] * mel_mx[i + 1];
                i += 2;
            }
        }

        for (i = 0; i < MEL; i++)
            mel[i] = intlog2_8bit(mel[i]);

        // remove noise floor and adjust it?....
        if (e < TOL_OFF)
            for (i = 0; i < MEL; i++) {
                mel[i] -= nfloor[i];
                if (mel[i] > 0 && nfloor[i] < 255)
                    nfloor[i]++;
                if (mel[i] < 0 && nfloor[i] > 0)
                    nfloor[i]--;
                if (mel[i] < 0)
                    mel[i] = 0;
            }
        else
            for (i = 0; i < MEL; i++) { // just remove noise floor....
                mel[i] -= nfloor[i];
                if (mel[i] < 0)
                    mel[i] = 0;
            }

        // calculate energy seperate from mdct (just sum-of-mels)
        e = 0;
        for (i = 0; i < MEL; i++)
            e += mel[i];
        // printf("e=%d\n",e);

        for (n = 0; n < CEPS; n++) {
            cep[n] = 0;
            for (i = 0; i < MEL; i++)
                cep[n] += mel[i] * dctm_8bit[n * MEL + i];
            cep[n] >>= 9; // only enough memory for buffering 8-bit ceps
        }

        if (e > TOL_ON) { // capture (another) audio frame...
            for (i = 0; i < CEPS; i++)
                word[wsize * CEPS + i] = (int8_t)cep[i];
            wsize++;
            silcount = 0;
        }
        if (e < TOL_OFF)
            silcount++; // silent frame, end of sample?
        if (silcount >= 20 && wsize < MINSIZE) {
            silcount = 0;
            wsize = 0;
        }
        lock = 0;

        if (wsize == WSIZE || (silcount >= 15 && wsize >= MINSIZE)) {
            // warp sample size to exactly 16 frames....
            if (wsize < 16) {
                for (i = 15; i >= 0; i--) {
                    k = i * wsize / 16;
                    for (n = 0; n < CEPS; n++)
                        word[i * CEPS + n] = word[k * CEPS + n];
                }
            }
            if (wsize > 16) {
                for (i = 0; i < 16; i++) {
                    k = i * wsize / 16;
                    for (n = 0; n < CEPS; n++)
                        word[i * CEPS + n] = word[k * CEPS + n];
                }
            }
            wsize = 16; // standardized now!

            // search codebook for best match and print it!
            bestdist = 999999999;
            c = 0;
            while (cb[c] != -1) {
                k = cb[c];
                c++;
                dist = 0; // printf("k=%d\n",k);
                for (i = 0; i < 16; i++) {
                    for (n = 0; n < CEPS; n++, c++) {
                        d = word[i * CEPS + n] - cb[c];
                        dist += d * d;
                    }
                }
                if (dist < bestdist) {
                    bestdist = dist;
                    best = k;
                }
            }
            printf("best match = %d, bestdist=%d\n", best, bestdist);
            wsize = 0;
            silcount = 0;
            posn = 0;
        }
        lastcount = count;
    }
}
