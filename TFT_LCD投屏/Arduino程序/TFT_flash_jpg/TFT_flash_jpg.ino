
#include <SPI.h>
#include <TFT_eSPI.h>
#include "WiFi.h"
#include "AsyncUDP.h"
#include <JPEGDecoder.h>
// Return the minimum of two values a and b
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

const char * ssid = "HONOR30";
const char * password = "12345678";
AsyncUDP udp;

TFT_eSPI tft = TFT_eSPI();


int asii_int(char c)
{
  if (c < 58)
//   if (c < 58 && c > 47)
  {
    return c-48;
  }
//  else if (c < 103 && c > 96)
   else if (c > 96)
  {
       return c-87;
  }
  else if (c < 81 && c > 64)
  {
    return c-64;
  }
  return 0;
}
void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos) {

  int x = xpos;
  int y = ypos;

  JpegDec.decodeArray(arrayname, array_size);

  renderJPEG(x, y);

}

//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void renderJPEG(int xpos, int ypos) {

  // retrieve infomration about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.readSwappedBytes()) {

    // save a pointer to the image block
    pImg = JpegDec.pImage ;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
    {
      tft.pushRect(mcu_x, mcu_y, win_w, win_h, pImg);
    }
    else if ( (mcu_y + win_h) >= tft.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }

  // calculate how long it took to draw the image
  drawTime = millis() - drawTime;

}

int cnt=0;
int d = 0;
char *dick = "ffd8ffe000104a46494600010100000100010000ffdb0043000201010101010201010102020202020403020202020504040304060506060605060606070908060709070606080b08090a0a0a0a0a06080b0c0b0a0c090a0a0affdb004301020202020202050303050a0706070a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0affc00011080080008003012200021101031101ffc4001f0000010501010101010100000000000000000102030405060708090a0bffc400b5100002010303020403050504040000017d01020300041105122131410613516107227114328191a1082342b1c11552d1f02433627282090a161718191a25262728292a3435363738393a434445464748494a535455565758595a636465666768696a737475767778797a838485868788898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7b8b9bac2c3c4c5c6c7c8c9cad2d3d4d5d6d7d8d9dae1e2e3e4e5e6e7e8e9eaf1f2f3f4f5f6f7f8f9faffc4001f0100030101010101010101010000000000000102030405060708090a0bffc400b51100020102040403040705040400010277000102031104052131061241510761711322328108144291a1b1c109233352f0156272d10a162434e125f11718191a262728292a35363738393a434445464748494a535455565758595a636465666768696a737475767778797a82838485868788898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7b8b9bac2c3c4c5c6c7c8c9cad2d3d4d5d6d7d8d9dae2e3e4e5e6e7e8e9eaf2f3f4f5f6f7f8f9faffda000c03010002110311003f00fdfca28c8eb9a323d6800a29372e71914b91eb400514647ad1907a11400514647ad14005141207534647ad00145146475cd0014526e5f514b91eb405d0514641e868c8f5a00f87be247fc153be21f80747f0ef88a6fd99a0b7b4d57c33a9ea3a9586b5e28b9b2d4edaeac63d4269228eca5d304be4dcdb69b73736b7373f66f3628a4f362b697cb8a5f48d0bf6fdf85d67fb3f788fe387c47bcd3746b8f0af88f54d0b5ed0b4fd722b9962beb1d57fb2c45e6cbf66f284b2c96b2f9b73e5451477d14b2cb1c47cdadebcfd97a3f889a2781e0f8d3f12bc41e25d53c05e25bed574fd6f4e9868b35f79915d5b431dc8b0f2f3e5db5cf972f95e50965889f2847218aaafc39fd967c57f0974ef13c1e17f8a62f2fbc49e27fed9d5aff5ad324bc92411456b6d6d1ffc7cf99ff1ed651097fe7acbe6f95f658bca8a2e4852c6b5ad43e9b1153876a51a74e10e49c377ef7bfeff00ff0023fdd343c09fb617c0af8a5f0c350f8afe08f1cc7afe9ba3de18f583e19b49af6e6da51298845e55ac72c9276fde4631247fbd8f319069ff00b2a7ed57a1fed59e065f16e81e1fd6f4db8b49e58f52b4bfd16fad85b7fa4cd1088cb756b1452dcc7f66923b98a1925fb2cbe645e6cbfba965d7d63e0aeb7e36d293c33f127c51677fa5c57b24b2e95a6e8e6dacb54865b1bab596d6fa2965b9fb55b9fb499445fbbfdec311e45617eca1fb1e7c2ffd92f47d5f4af01784b49825d775cd42f754d574cd2a3b2325b497f7573696b24718e7ecd15efd9a33ff003ca21c460f963487d67db43f90e0c47f622c0d470e75539e1c9f6fddebd0e6bf696ff8293fecf1fb2f7c45d57e197c4e4d462d7b4bf08c7e26b7d3ece38cff006a5af9b2c72456e65923f36e631149298b8fdd7ef39f2e5f2b535bfdba3e13785fe2e7887e0ec9a06b525f785fc397daadf6a1696911b590dbc56173716b1e65129b9f2b52b397fd5f95fbdff5b98a4c667ed4bff04eaf83ff00b54ffc242df107c41afd85e788b4cd22c7fb434996247b1fb0cb7db25b632c52fef258efae6297ccf33f752e2b9fd43fe0939fb33eb1e2c93c7fa3bea3a378a2eb5ed5f53bdf12e9c96a2f2f7fb4bedd1dcdb4a658a58cc7e5df4918fddf9b88a2fde706b0a9f5ef6dfbbf80f6a843823ea34fdbcea7b4fb7fe2b4393f1e7fc342a7873fe0ae5fb3b7c46f0daf8a7e177853c59aec8744d1b51b6d3ececad629669352d63fb222b4cdd5d451c57115d732f99fbaf2bf7914b2f22bd53f66efda9345f8f5e27f16fc3ad4bc2fac784bc65e09bbb183c4fe15d6a5b5965b617311b9b696396d659639629223274933fb99331c75e5fe12ff00824b7ecd5e035be83c3379e29b5bbd53c25a1e8771731ea30fee66d20db1b1d523fdd7fc7f47f66b6ffa76ff0046ff0052332f9bd4786fe017c49fd9b358d63c65f05b44d37e23f8a7c71a94771e34f1578efc551e877d2fd9ada2b5b18a2fecfd1a58a58e38a393f77e5c5e51f33fd6f9b2d287d75a84e64e610e13a94ea432ee7e7fb1cfdfdcff00b7793e3ecefcbe852d4bfe0a19e1bb5fdac75bfd94f48f85be20d6f5bd10cb1cd269d7da5c4f";
uint8_t Baboon[16000] = {0};
uint8_t temp[2000] = {0};
void setup() {
  Serial.begin(115200);
  tft.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  if (udp.listen(1234)) {
    Serial.println("UDP connected");
    udp.onPacket([](AsyncUDPPacket packet) {
      if (packet.data()[0] == ';') {
        for (int i = 0; i < packet.length() / 2; i++)
        {
          Baboon[i] = asii_int(packet.data()[1 + 2 * i]) * 16 + asii_int(packet.data()[1 + 2 * i + 1]);
        }

      }

      else   if (packet.data()[0] == '/') {
        for (int i = 0; i < packet.length() / 2; i++)
        {
          Baboon[d * 512 + 512 + i] = asii_int(packet.data()[1 + 2 * i]) * 16 + asii_int(packet.data()[1 + 2 * i + 1]);
        }
        
        drawArrayJpeg(Baboon, sizeof(Baboon), 4, 0); // Draw a jpeg image stored in memory
//        if(cnt%10==0){
//        int ti = millis();
//        char a[20]={0};
//        sprintf(a,"%dq",ti);
//            udp.broadcastTo(a, 1234);}
//         cnt++;
      }
      else
      {
        d = asii_int(packet.data()[0]);
        for (int i = 0; i < packet.length() / 2; i++)
        {
          Baboon[d * 512 + i] = asii_int(packet.data()[1 + 2 * i]) * 16 + asii_int(packet.data()[1 + 2 * i + 1]);
        }
      }
    });
    //Send unicast
//    udp.print("Hello Server!");
  }
   int c =0;
  for(int a=0;a<20000;a++)
 { if(a==0)
{int t1i = millis();
        char aa[20]={0};
        sprintf(aa,"%dstart",t1i);
            udp.broadcastTo(aa, 1234);}
    if(a==19999)
{int t1i = millis();
        char aa[20]={0};
        sprintf(aa,"%dend",t1i);
            udp.broadcastTo(aa, 1234);}        
  c= asii_int('a');}
}

void loop() {

    // tft.setRotation(0);  // portrait
    // tft.fillScreen(TFT_BLACK);
    //
    //  drawArrayJpeg(EagleEye, sizeof(EagleEye), 0, 16); // Draw a jpeg image stored in memory at x,y
    //  delay(2000);
    //  tft.setRotation(0);  // portrait
    //  tft.fillScreen(TFT_BLACK);
    //  drawArrayJpeg(Baboon, sizeof(Baboon), 4, 0); // Draw a jpeg image stored in memory
  delay(3000);
  

   
}
