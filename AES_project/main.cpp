#include <iostream>
#include <fstream>
#include <cmath>
#include <bitset>
#include <stdio.h>
#define mod 0b101001101 //기약 다항식
using namespace std;
#define RightCircularShift(data,num,nbit) (((data)>>(num)) | ((data)<<(nbit-(num))))//nbit비트크기 형 데이터를 num비트 만큼 Right circular shift해주는 매크로이다.
#define LeftCircularShift(data,num,nbit) (((data)<<(num)) | ((data)>>(nbit-(num))))//nbit비트크기 데이터를 num비트 만큼 Right circular shift해주는 매크로이다.
#define Index(Data,Row,Column) (((unsigned char*)(Data + Row))[Column]) //int형으로 포장된 객체를 char형으로 읽어서 인덱스를 접근해주는 매크로
#define For(m,n,i,j) for(int i = 0; i <m; ++i)for(int j = 0; j < n; ++j) //이중for문을 간단히 사용하기 위한 매크로
#define COUT {cout.width(2); cout << hex;} cout //width와 hex설정을 간편하게 하기 위해 만든 출력 매크로
#define Print(data) For(4, 4,x,y){ COUT <<(unsigned int)(data)[y][x] << " "; }cout<<"\n"//반복되는 출력을 간단히 사용하기 위한 매크로
#define SubBytes(data) 	for (int j = 0; j < 4; ++j) (data)[j] = SubWord((data)[j])//Subword는 32비트 씩 바꿀 수 있으므로 128비트를 바꾸기 위해 for문을 돌려주는 매크로이다.
#define inverseSubBytes(data) 	for (int j = 0; j < 4; ++j) (data)[j] = inverseSubWord((data)[j])//inverseSubword는 32비트 씩 바꿀 수 있으므로 128비트를 바꾸기 위해 for문을 돌려주는 매크로이다.
int degree(unsigned int num) {//차수를 구한다.
	for (int i = 8; i >= 0; --i)
		if ((num & (1 << i)) != 0) return i;
}
unsigned char reduce(unsigned int num) {//기약다항식을 기준으로 차원을 낮춘다.
	int temp = degree(num) - 8;
	if (temp >= 0) {
		return ((unsigned int)mod << temp) ^ num;
	}
	return num;
}
void swap(unsigned int &x, unsigned int &y) {//x,y를 교환한다.
	unsigned char temp = x;
	x = y;
	y = temp;
}

class GaloisNumber {
private:
	unsigned char num = 0;//최고차항은 7차이므로  8비트인 char타입으로 갈루아 체의 원소를 표현하자.
public:
	GaloisNumber(){};//기본 생성자
	GaloisNumber(unsigned char data):num(data){};//생성자
	unsigned char getValue() {
		return num;
	}
	void setValue(unsigned char num) {//반드시 unsigned char형으로 파라미터를 넣어줘야한다. 8비트 자료형보다 크면 값이 잘릴 수 있다.
		this->num = num;
	}
	GaloisNumber xtimes(void) {//자신에게 x를 곱한 결과를 리턴한다.
		unsigned int temp = this->num;//x를 곱할 경우, 8차가 넘어가며 캐리가 발생할 수 있기 때문에, 여유 비트를 확보하기 위해 int형에 넣는다.
		return GaloisNumber((temp << 1) ^ ((((temp >> 7)&1)*(unsigned int)mod)));
	}
	GaloisNumber operator+(const GaloisNumber &data) {
		return GaloisNumber(this->num^data.num);//덧셈은 비트별로 xor연산을 해주면 된다.
	}
	GaloisNumber operator-(const GaloisNumber &data) {
		return GaloisNumber(this->num^data.num);//뺄셈도 비트별로 xor연산을 해주면 된다.
	}
	GaloisNumber operator*(const GaloisNumber &data) {
		GaloisNumber result(0);
		GaloisNumber temp = data;
		for (int i = 0; i < 8; ++i) {
			result = result + GaloisNumber(((this->num >> i) & 1)*temp.num);//각 비트를 체크하여 곱해지는 수에 계속 x를 곱해가며 더해준다. 즉, 곱셈은 내부적으로 shift와 xor을 이용한다.
			temp = temp.xtimes();
		}
		return result;
	}


	GaloisNumber inverse() {
		if (this->num == 0) return GaloisNumber(0); //0일 경우는 0을 리턴해준다.
		unsigned int temp1 = this->num, v = 0;
		unsigned int M = mod, u = 1;
		while (temp1 != 1) {//gcd가 1이 나올 때 까지 반복한다.
			int j = degree(M)-degree(temp1);//차수의 차이만큼 몫이 발생할 것이다.
			if (j < 0) {//차수의 차이가 음수라면 스왑 해준다.
				swap(temp1, M); swap(u,v); j = -j;
			}
			M = M ^ (temp1<< j); //몫만큼 x를 곱해서 빼준다.
			v = v^ (u<< j);//몫만큼 x를 곱해서 빼준다.
		}
		return GaloisNumber(reduce(u));//32비트 정수형으로 표현된 다항식을 모듈러로 차수를 낮춘 다음 리턴한다.
	}

};
void AES_Encypt(unsigned char plain[][4], unsigned char key[][4]);
void AES_Decypt(unsigned char cipher[][4], unsigned char key[][4]);
void transpose(unsigned int* data);
unsigned int SubWord(unsigned int input);
void ShiftRow(unsigned int* data);
void MixColumn(unsigned int* data);
void AddRoundKey(unsigned int* data, unsigned int* roundkey);
void KeyExpansion(unsigned int* input, unsigned int* output, const int &round);
unsigned int Rfunction(unsigned int input, const int &round);
unsigned int inverseSubWord(unsigned int input);
void inverseShiftRow(unsigned int* data);
void inverseMixColumn(unsigned int* data);
void set_sbox();
void setInverse_sbox(unsigned char inverse_sbox[][16]);



//Sbox를 미리 담아두자.
unsigned char sbox[16][16];
unsigned char inverse_sbox[16][16];
//RC값을 미리 담아두자.
unsigned char RC[10] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x4D,0x9A };
GaloisNumber MixTable[4][4]=
{{0x02,0x03,0x01,0x01},
 {0x01,0x02,0x03,0x01},
 {0x01,0x01,0x02,0x03},
 {0x03,0x01,0x01,0x02}};
GaloisNumber inverseMixTable[4][4] =
{ {0x0E,0x0B,0x0D,0x09},
 {0x09,0x0E,0x0B,0x0D},
 {0x0D,0x09,0x0E,0x0B},
 {0x0B,0x0D,0x09,0x0E} };


unsigned int* roundkey[11];
int main() {
	set_sbox();	cout.fill('0');
	cout << "RC:";
	for (int i = 0; i < sizeof(RC) / sizeof(unsigned char); ++i) { COUT << (unsigned int)RC[i] << " "; }

	ifstream plainfile("plain.bin", ios::in | ios::binary);//파일을 오픈한다
	unsigned char plain[4][4];//버퍼를 선언한다
	For(4, 4, i, j) plainfile.read((char*)&plain[j][i], 1);//읽어들인다.
	plainfile.close();//파일을 닫는다

	ifstream keyfile("key.bin", ios::in | ios::binary);//파일을 오픈한다
	unsigned char key[4][4];//버퍼를 선언한다
	For(4, 4, i, j) keyfile.read((char*)&key[j][i], 1);//읽어들인다
	keyfile.close();//파일을 닫는다

	cout << "\nPLAIN: "; Print(plain);
	cout << "KEY: "; Print(key);
	cout << "\n<------ENCRYPTION------>\n";


	AES_Encypt(plain, key);//암호화를 실행한다.
	cout << "\nCIPHER :"; Print(plain);//완료된 암호문을 출력한다.

	ofstream cipherfile("cipher.bin", ios::out | ios::binary);//파일을 오픈한다.
	For(4, 4, i, j) cipherfile.write((char*)&plain[j][i], 1);//완성된 암호문을 쓴다.
	cipherfile.close();

	cout << "\n<------DECRYPTION------>\n";
	setInverse_sbox(inverse_sbox);//inverse sbox를 계산한다.
	AES_Decypt(plain, key);//복호화를 실행한다.
	cout << "\nDECRYPTED: "; Print(plain);//완료된 평문을 출력한다.

	ofstream decryptfile("decrypt.bin", ios::out | ios::binary);//파일을 오픈한다.
	For(4, 4, i, j) decryptfile.write((char*)&plain[j][i], 1);//완성된 암호문을 쓴다.
	decryptfile.close();
	system("pause");
}

void AES_Encypt(unsigned char plain[][4], unsigned char key[][4]) {
	////////키 생성 과정////////
	cout << "\nKEY EXPANSION\n";

	for (int i = 0; i < 11; ++i) roundkey[i] = new unsigned int[4];
	cout << "Round 0: ";
	For(4, 4, x, y) {
		Index(roundkey[0], y, x) = key[y][x];
		COUT << (unsigned int)Index(roundkey[0], y, x) << " ";
	}
	cout << "\n";
	for (int i = 0; i < 10; ++i) {
		KeyExpansion(roundkey[i], roundkey[i + 1], i + 1);
		cout << "Round " << dec << i + 1 << ": ";
		For(4, 4, x, y) { COUT << (unsigned int)Index(roundkey[i + 1], y, x) << " "; }
		cout << "\n";
	}
	////////0~10라운드////////
	cout << "\nRound 0\n";
	AddRoundKey((unsigned int*)plain, roundkey[0]);
	cout << "AR: ";	Print(plain);

	for (int i = 1; i < 10; ++i) {
		cout << "\nROUND " << dec << i << "\n";

		SubBytes((unsigned int*)plain);
		cout << "SB: "; Print(plain);
		ShiftRow((unsigned int*)plain);
		cout << "SR :"; Print(plain);
		MixColumn((unsigned int*)plain);
		cout << "MC :"; Print(plain);
		AddRoundKey((unsigned int*)plain, roundkey[i]);
		cout << "AR :"; Print(plain);
	}

	cout << "\nROUND 10" << "\n";

	SubBytes((unsigned int*)plain);
	cout << "SB: "; Print(plain);

	ShiftRow((unsigned int*)plain);
	cout << "SR :"; Print(plain);

	AddRoundKey((unsigned int*)plain, roundkey[10]);
	cout << "AR :"; Print(plain);
}
void AES_Decypt(unsigned char cipher[][4], unsigned char key[][4])
{
	////////0~10라운드////////
	cout << "\nRound 0\n";
	AddRoundKey((unsigned int*)cipher, roundkey[10]);
	cout << "AR: ";	Print(cipher);

	for (int i = 1; i < 10; ++i) {
		cout << "\nROUND " << dec << i << "\n";
		inverseShiftRow((unsigned int*)cipher);
		cout << "SR :"; Print(cipher);
		inverseSubBytes((unsigned int*)cipher);
		cout << "SB: "; Print(cipher);
		AddRoundKey((unsigned int*)cipher, roundkey[10-i]);
		cout << "AR :"; Print(cipher);
		inverseMixColumn((unsigned int*)cipher);
		cout << "MC :"; Print(cipher);

	}

	cout << "\nROUND 10" << "\n";

	inverseShiftRow((unsigned int*)cipher);
	cout << "SR :"; Print(cipher);

	inverseSubBytes((unsigned int*)cipher);
	cout << "SB: "; Print(cipher);

	AddRoundKey((unsigned int*)cipher, roundkey[0]);
	cout << "AR :"; Print(cipher);
}
void set_sbox()//sbox는 파라미터로 받지 말고 global로 둬서 계산하자.
{
	GaloisNumber temp;
	For(16, 16, i, j) {
		sbox[i][j] = (i << 4) | j; //먼저 차례대로 값을 채우자.
		temp.setValue(sbox[i][j]);
		sbox[i][j] = temp.inverse().getValue(); //역원을 구해서 대입하자.
		sbox[i][j] = (sbox[i][j]) ^ (LeftCircularShift(sbox[i][j], 1,8)) ^ (LeftCircularShift(sbox[i][j], 2,8)) ^ (LeftCircularShift(sbox[i][j], 3,8)) ^ (LeftCircularShift(sbox[i][j], 4,8)) ^ 0x15;
	}
}
void setInverse_sbox(unsigned char inverse_sbox[][16])//sbox에서 찾아서 inverse를 만들자.
{
	For(16, 16, i, j) {
		for (int x = 0; x < 16; ++x) {
			for (int y = 0; y < 16; ++y) {
				if (sbox[x][y] == (unsigned char)((i << 4) | j)) {
					inverse_sbox[i][j] = (unsigned char)((x << 4) | y);
					goto stop;
				}
			}
		}
	stop:;
	}
}
;
void transpose(unsigned int* data)//입력으로 들어온 int[4]배열을 char[4][4]로 해석하여 전치행렬을 구한다.
{
	unsigned char temp;
	for (int i = 0; i <3; i++)
		for (int j = i + 1; j < 4; j++) {
			temp = Index(data, i, j);
			Index(data, i, j) = Index(data, j, i);
			Index(data, j, i) = temp;
		}
}

void MixColumn(unsigned int * data)//unsigned int[4]배열을 파라미터로 받아서 MixColum를 수행하자.
{
	//갈루아 체 곱셈을 수행하기 위해 데이터를 갈루아 객체에 넣자
	GaloisNumber input[4][4];
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			input[i][j] = (((unsigned char*)data) + 4 * i)[j];

	GaloisNumber output[4][4];
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) {
			for (int n = 0; n < 4; ++n)	output[i][j] = output[i][j] + (MixTable[i][n] * input[n][j]); //행렬곱을 수행한다.
			(((unsigned char*)data) + 4 * i)[j] = output[i][j].getValue();//수행한 뒤 바로 업데이트 한다.
		}
}

unsigned int SubWord(unsigned int input)
{//미리 저장된 Sbox로 부터 값을 찾아 리턴한다.
	unsigned int output = 0;
	for (int i = 0; i < 4; ++i) {
		//캐릭터 타입을 이용해 바이트 단위로 접근하여 인덱스를 알아낸다.
		((unsigned char*)&output)[i] |= sbox[(((unsigned char*)&input)[i] >> 4) & 0b1111][(((unsigned char*)&input)[i]) & 0b1111];
	}
	return output;
}

unsigned int Rfunction(unsigned int input, const int &round)//input은 4바이트 이므로 unsigned int형으로 받아준다.
{
	input = RightCircularShift(input, 8,32);//8비트만큼 shift를 해준다. 명령어는 right shift이지만, 내부적으로 메모리가 반전되어 있어서 실제로는 데이터를 left shift해주는 효과가 있다.
	input = SubWord(input); //전체적으로 Sbox를 통과시킨다.
	((unsigned char*)&input)[0] = (((unsigned char*)&input)[0] ^ RC[round-1]);//RC값을 xor해준다.
	return input;
}

unsigned int inverseSubWord(unsigned int input)
{//미리 저장된 inverse_Sbox로 부터 값을 찾아 리턴한다.
	unsigned int output = 0;
	for (int i = 0; i < 4; ++i) {
		//캐릭터 타입을 이용해 바이트 단위로 접근하여 인덱스를 알아낸다.
		((unsigned char*)&output)[i] |= inverse_sbox[(((unsigned char*)&input)[i] >> 4) & 0b1111][(((unsigned char*)&input)[i]) & 0b1111];
	}
	return output;
}

void inverseShiftRow(unsigned int * data)
{
	for (int i = 0; i < 4; ++i) data[i] = LeftCircularShift(data[i], 8 * i,32);
}

void inverseMixColumn(unsigned int * data)
{
	//갈루아 체 곱셈을 수행하기 위해 데이터를 갈루아 객체에 넣자
	GaloisNumber input[4][4];
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			input[i][j] = (((unsigned char*)data) + 4 * i)[j];

	GaloisNumber output[4][4];
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) {
			for (int n = 0; n < 4; ++n)	output[i][j] = output[i][j] + (inverseMixTable[i][n] * input[n][j]); //행렬곱을 수행한다.
			(((unsigned char*)data) + 4 * i)[j] = output[i][j].getValue();//수행한 뒤 바로 업데이트 한다.
		}
}

void KeyExpansion(unsigned int* input,unsigned int* output,const int &round)//입력으로 받은 키를 이용해 다음 라운드 키를 생성하여 반환한다.
{
	if (round == 0) {
		memcpy(output, input,16);
		return;
	}
	transpose(input);//열 계산을 행 계산으로 간편하게 하기 위해 전치시킨다.
	output[0] = Rfunction(input[3],round) ^ input[0];
	output[1] = output[0] ^ input[1];
	output[2] = output[1] ^ input[2];
	output[3] = output[2] ^ input[3];
	transpose(output);
	transpose(input);
}

void AddRoundKey(unsigned int * data, unsigned int * roundkey)
{
	for (int i = 0; i < 4; ++i) data[i] = data[i] ^ roundkey[i];
}

void ShiftRow(unsigned int* data)//unsigned int[4]배열을 파라미터로 받아서 ShiftRow를 수행하자.
{
	for (int i = 0; i < 4; ++i) data[i] = RightCircularShift(data[i], 8 * i,32);
}
