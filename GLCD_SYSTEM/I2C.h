/*
 * I2C.h
 *
 * Created: 2017/12/28 20:19:50
 *  Author: kodai
 */ 


#ifndef I2C_H_
#define I2C_H_

void InitI2c(void){
	TWSR = 0b00000000; // N=1
	TWBR = 255;         // 100kHz = Fosc/(16+2*TWBR*N) @Fosc=8MHz, N=1
}
/*I2Cのクロック周波数設定を行います。
TWSRでクロック設定用パラメータNを指定します。
TWBR、N、Fosc(F_CPU)の値でI2Cのクロックが決定します。なお各レジスタの詳細は、ターゲット・マイコンのデータシートで確認します。*/
//Start I2C Command
//Start with write bit: StartI2c( (7bit Slave Address << 1) | TW_WRITE )
//Start with read bit: StartI2c( (7bit Slave Address << 1) | TW_READ )
unsigned char StartI2c(unsigned char dataStartI2c){
	unsigned char stateOfTWI;
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	while( !(TWCR & _BV(TWINT)) );
	
	stateOfTWI = TWSR & TW_STATUS_MASK;
	if( stateOfTWI != TW_START && stateOfTWI != TW_REP_START ){
		return 0;
	}
	
	TWDR = dataStartI2c;
	TWCR = _BV(TWINT) | _BV(TWEN);
	while( !(TWCR & _BV(TWINT)) );
	
	stateOfTWI = TWSR & TW_STATUS_MASK;
	if( stateOfTWI != TW_MT_SLA_ACK && stateOfTWI != TW_MR_SLA_ACK ){
		return 0;
	}
	return 1;
}
//Stop I2C
void StopI2c(void){
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
	while( !(TWCR & _BV(TWSTO)) );
}
//Transmit data form master
unsigned char TransI2c(unsigned char dataTransI2c){
	TWDR = dataTransI2c;
	TWCR = _BV(TWINT) | _BV(TWEN);
	while( !(TWCR & _BV(TWINT) ) );
	
	if( (TWSR & TW_STATUS_MASK) != TW_MT_DATA_ACK){
		return 0;
	}
	return 1;
}
//Recieve data from salve, with ACK
unsigned char RecieveAckI2c(void){
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
	while( !(TWCR & _BV(TWINT)) );
	return TWDR;
}
//Recieve data from slave, with NACK
unsigned char RecieveNakI2c(void){
	TWCR = _BV(TWINT) | _BV(TWEN);
	while( !(TWCR & _BV(TWINT)) );
	return TWDR;
}




#endif /* I2C_H_ */