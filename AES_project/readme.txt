1) 구현환경 및 구현언어
C++를 이용해 비주얼 스튜디오에서 콘솔 프로그램으로 구현하였다.
2) AES 암호화
가장 먼저 아래의 갈루아 체 구조체를 구현하였다.

class GaloisNumber {
private:
	unsigned char num ;//최고차항은 7차이므로  8비트인 char타입으로 갈루아 체의 원소를 표현하자.
public:
	GaloisNumber();//기본 생성자
	GaloisNumber(unsigned char data);//생성자
	unsigned char getValue();//get함수
	void setValue(unsigned char num); //set함수
	GaloisNumber xtimes(void);//자신에게 x를 곱한 결과를 리턴한다.
	GaloisNumber operator+(const GaloisNumber &data); //+오버로딩.+는 결국 xor이다.
	GaloisNumber operator-(const GaloisNumber &data); //-오버로딩.-는 결국 +와 같다..
	GaloisNumber operator*(const GaloisNumber &data); //*오버로딩.*는 xor,shift 사용.
	GaloisNumber inverse();//확장된 유클리드 알고리즘을 사용한 역원 구하는 함수
}


그 다음 sbox를 생성하기 위한 아래의 함수를 구현하였다. Sbox는 아래와 같이 unsigned char배열을 이용하여 글로벌에 선언해두기 때문에 따로 인자를 받지 않는다.

void set_sbox();
unsigned char sbox[16][16];


Sbox()를 생성한 뒤에는 먼저 plain.bin과 key.bin에서 바이너리 데이터를 읽어들여서 아래의 두 버퍼에 담는다.

unsigned char plain[4][4];
unsigned char key[4][4];


다음으로 키를 생성한다. 키 생성에는 아래의 세 함수가 사용된다. 키 확장은 각 칼럼을 R함수와 xor을 이용하여 연산해 다음 라운드 키를 생성해낸다.

void KeyExpansion(unsigned int* input, unsigned int* output, const int &round);
unsigned int Rfunction(unsigned int input, const int &round);
void transpose(unsigned int* data); //계산 편의상 행렬을 전치시켜주는 함수


위에서 생성해낸 key들은 아래의 배열에 담았다. 글로벌로 접근하기 위해 메인 바깥에 선언해두었다. 0번 인덱스에는 오리지널 key가 들어가며, 그 다음부터 확장된 key들이 들어간다.

unsigned int* roundkey[11];


0번째 라운드에서는 우선 아래의 함수를 이용해 AddRoundKey를 수행한다. 아래 함수는 그저 파라미터로 받은 라운드 key와 data를 xor해주기만 하는 간단한 함수이다.

void AddRoundKey(unsigned int* data, unsigned int* roundkey);


그 후 1에서 9라운드 까지는 아래의 네 개의 함수를 차례대로 반복한다.

#define SubBytes(data) 	for (int j = 0; j < 4; ++j) (data)[j] = SubWord((data)[j])
void ShiftRow(unsigned int* data);
void MixColumn(unsigned int* data);
void AddRoundKey(unsigned int* data, unsigned int* roundkey);


첫번 째로 SubBytes함수는 이미 만들어 두었던 아래의 Subword라는 함수를 재활용하기 위해, 간단하게 매크로로 구현하였다.

unsigned int SubWord(unsigned int input);


Subword는 4바이트 자료형을 파라미터로 받는다.
즉, 칼럼 한 개를 입력으로 받아 Sbox를 이용해 치환해주는 함수이다.
따라서 4개의 칼럼 전체를 치환해주기 위해 위에서 언급한 것 처럼 SubBytes라는 매크로 함수를 간단히 for문을 이용해 구현하였다.
두번째 shiftrow는 각 행마다 circularshift를 수행하는 함수이므로 구현을 위해 아래와 같은 매크로를 정의해 이용하였다.
만약을 위해 아래와 같이 left와 right 모두 정의해 놓았다.

#define RightCircularShift(data,num,nbit) (((data)>>(num)) | ((data)<<(nbit-(num))))
#define LeftCircularShift(data,num,nbit) (((data)<<(num)) | ((data)>>(nbit-(num))))


세번째 MixColumn은 이미 만들어둔 갈루아 체 구조체 배열을 이용해 행렬곱을 수행하였다.
이미 갈루아체 곱셈과 덧셈 연산자를 오버로드해서 구현했으므로, 단순히 행렬 곱만 구현하면 되어서 쉽게 구현할 수 있었다.
사용된 행렬은 아래와 같다.

GaloisNumber MixTable[4][4]=
{{0x02,0x03,0x01,0x01},
 {0x01,0x02,0x03,0x01},
 {0x01,0x01,0x02,0x03},
 {0x03,0x01,0x01,0x02}};


그리고 10번째 라운드에서는 아래 세개의 함수만을 이용해 라운드를 진행하였다.
#define SubBytes(data) 	for (int j = 0; j < 4; ++j) (data)[j] = SubWord((data)[j])
void ShiftRow(unsigned int* data);
void AddRoundKey(unsigned int* data, unsigned int* roundkey);

이렇게 암호화된 암호문을 ‘cipher.bin’에 출력하도록 구현하였다.


3) AES 복호화
위에서 구현해놓은 암호화 함수들을 약간만 수정하니 손쉽게 복호화를 구현할 수 있었다.
복호화를 시작하기에 앞서서 가장 먼저 아래의 함수를 이용해 inverse sbox를 생성하였다.
복호화는 아래와 같은 함수로 진행하였다.

void setInverse_sbox(unsigned char inverse_sbox[][16]);

위 함수는 기존의 sbox에서 값을 일일히 찾아서 inverse sbox를 생성해주는 함수이다.

그렇게 생성한 inverse sbox는 전역적으로 선언한
unsigned char inverse_sbox[16][16];
변수에 담았다.

복호화에는 키를 생성하는 부분은 제외시켰다.
왜냐하면 이미 암호화를 수행하고 난 상태이기 때문에 ,
unsigned int* roundkey[11];
변수에 확장된 키들이 들어있을 것이기 때문이다.
복호화는 아래의 함수를 통해 이루어진다.

void AES_Decypt(unsigned char cipher[][4], unsigned char key[][4]);

AES_Decypt()의 첫번째 인자는 암호문을 받는 부분이며, 두번째 인자는 키를 받는 부분이다.
사실 이미
roundkey[11];
가 전역적으로 선언되어 있기에 굳이 키를 받을 필요는 없었고,
실제로 함수 내부에서도 전혀 사용되지도 않았다.
하지만 암호화를 하지 않고, 복호화를 먼저 수행할 경우에는 복호화 내부에서도 키 확장을 하여 키를 구해 놓아야 하기 때문에
일단 인자로 받아 두기는 했다.
하지만 이번 프로젝트에서는 굳이 복호화에 키 확장 알고리즘을 넣을 필요가 없었기 때문에 구현하지는 않았다.
AES_Decypt()는 총 10개의 라운드로 진행이 된다. 마지막 라운드를 제외한 모든 라운드는 아래의 함수들을 반복한다.

void inverseShiftRow(unsigned int* data);
#define inverseSubBytes(data) for (int j = 0; j < 4; ++j) (data)[j] = inverseSubWord((data)[j])
void AddRoundKey(unsigned int* data, unsigned int* roundkey);
void inverseMixColumn(unsigned int* data);

물론 첫 라운드가 시작되기 전에 AddRoundKey()를 한번 수행해준다.
이때 사용되는 AddRoundKey()함수는 암호화를 사용했을 때와 정확히 같은 함수이다.
inverseShiftRow()는 이전에 혹시 몰라서 같이 정의해 두었던 LeftCircularShift() 매크로를 이용해 간단히 구현하였다.
그저 기존의 ShiftRow()함수에서 Right를 Left로 바꾸기만 하면 되는 것이었다.
그리고 다음으로 inverseSubBytes()도 매크로를 이용해 정의하고,
실질적인 inverse sbox과정은 inverseSubWord()함수를 이용해 구현하였다.
inverseMixColumn()또한 이미 계산된 역행렬를 곱해주기만 하면 되므로 간단하게 구현할 수 있었다.
사용된 행렬은 아래와 같다.

GaloisNumber inverseMixTable[4][4] =
{ {0x0E,0x0B,0x0D,0x09},
 {0x09,0x0E,0x0B,0x0D},
 {0x0D,0x09,0x0E,0x0B},
 {0x0B,0x0D,0x09,0x0E} };

그리고 마지막 라운드에는 inverseShiftRow,inverseSubBytes,AddRoundkey과정을 수행하여 복호화를 마쳤다.
이렇게 완료된 평문은 decrypt.bin에 바이너리 파일로 출력되도록 하였다.
