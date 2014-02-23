/***************************************************************************************************************
name:		RSA.C
author:		yuxiang
date:       2011.9.21
note��		������ʹ�õ���RSA 512λ�����㷨��Ĭ��Ϊ��ԿN����Ϊ64���ֽڣ����ֽ�Ϊ��Ԫ�洢��
			���б䶯���뼰ʱ�޸�RSA.h�е�NSIZE��UNIT_BITS���Լ���������������������(��UNIT_BITS�Ķ�)��
			
*************************************************************************************************************/

#include<stdio.h>
#include<string.h>
#include "RSA.h"

/******************�Ƚ�����***************************************/
/*byte1>byte2,1;byte1<byt2,-1;byte1=byte2 0;*/

int Large_Compare(U8*largebyte1,U8*largebyte2)
{	
	U8 i=0;
	U8 len=NSIZE;
    i=len;
    largebyte1+=len;                    //��ָ�����������ĸ�λ�ֽ�
    largebyte2+=len;
	while (i--)
		if (*--largebyte1 - *--largebyte2)
			return (int) *largebyte1 - (int) *largebyte2;
	return 0;

}
/****************��������***b-->a***a=b**************************/
void Large_Copy(U8*a,U8*b)
{
	U16 i = NSIZE;
	while (i--)
		*a++ = *b++;
}

/****************��������******a =a*2=a+a ***********************/
U16 Large_Slift(U8*a)
{
	 U16 c = 0, i = NSIZE;
	while (i--) 
	{
		c |= (U16)* a << 1;
		*a++ = c;
		c = (c >>UNIT_BITS)&1;
	}
	if((c&0xffff)!=0)
	{
		printf("Large_Slift:���λ�н�λ��\n");
	}
	return c;
}
/****************��������******a=a-b*****************************/
U16 Large_Sub(U8*a,U8*b)
{
	U16 c = 0, i = NSIZE;
	while (i--) 
	{
		c = *a - *(b++) - c;
		*a++ = c;
		c = (c >> UNIT_BITS) &1;          //����λ�н�λ��ʱ��c=0x0001
	}
	if(c&0xffff!=0)
	{
			printf("Large_Sub:���λ�н�λ!\n");
	}
    return c;
}

/****************�ӷ�����****** a=a+b ***************************/
U16 Large_Add(U8*a,U8*b)
{
	U16 c = 0, i = NSIZE;
	while (i--) 
	{
		c = *(b++) + *a + c;
		*a++ = c;
		c >>= UNIT_BITS;
	} 
	if(c&0xffff!=0)
	{
    	printf("Large_Add:���λ�н�λ��\n");
	}
    return c;
}
/**************************��ģ����******a=a*b mod N***************/
void Mul_Mod(U8*a,U8*b,U8*Np)
{
	U16 k =UNIT_BITS;
	U16 i;
	U8 temp[NSIZE];
	memset(temp,0,NSIZE);
	for(i=NSIZE;i>0;i--)
	{
		for(k=UNIT_BITS;k>0;k--)
		{
		  Large_Slift(temp);                 //temp=temp+temp;
		 if(Large_Compare(temp,Np)>0)        //temp>Nʱ	      
			   Large_Sub(temp,Np);           // temp=temp-N;
		 if((a[i-1]&(1<<(k-1)))!=0)          //��Ӧλ��ֵΪ1
			  Large_Add(temp,b);             //temp=temp+b;
		 if(Large_Compare(temp,Np)>0)        //temp>Nʱ
			  Large_Sub(temp,Np);            // temp=temp-N;
		}
	}
    Large_Copy(a,temp);                      //���ֵ����a
	
}
/*******************************��ģ����****************************/
void Rec_Power_Mod(U8*p_output,U8*p_input,U8*ped_byte,U8*pN,U8 ed_len)
{
U16 k=UNIT_BITS;                  
U16 i=0;							         //i�洢��d��ռ���ֽ���
U8  dectemp[64];
memset(dectemp,0,64);
if(ped_byte[ed_len-1]&(1<<7))				 //D������ֽڵ����λ�Ƿ�Ϊ1
	Large_Copy(dectemp,p_input);		     //p_dectemp=p_input
else
	dectemp[0]=0x01;					     //dectemp=1;
for(i=ed_len;i>0;i--)
	for(k=UNIT_BITS;k>0;k--)
	{
		Mul_Mod(dectemp,dectemp,pN);         //dectemp=(dectemp*dectemp)mod (pN);
		if((ped_byte[i-1]&(1<<(k-1)))!=0)    //��Ӧλ��ֵΪ1
	    Mul_Mod(dectemp,p_input,pN);	     //dectemp=(dectemp*p_input)mod(pN);
	}

 Large_Copy(p_output,dectemp);   

}



/****************************************RSA���ܺ���**********************************
U8*p_decrypted 	���ܺ�������ַ�����           ���������
U8* p_crypted 	�����ܵ������ַ�����           �����룩��
U8*p_dbyte     	������Կd                       (����)��
U8*p_Nbyte     	��Կ����N                      �����룩��
U16 pdec_len    �洢���ĵ�����p_decrypted�ĳ��ȣ����룩��
U16 pec_len     ����������p_crypted�ĳ���      �����룩��
U16 pd_len      ����p_dbyte�ĳ���              �����룩��
U16 pN_len      ����p_Nbyte�ĳ���              �����룩��
����ֵ�� 
 -2            	����: ��������ָ��Ϊ�ջ����鳤�Ȳ������㣻
 -1            	�������ĳ���pec_len����N�ĳ��ȣ�
  0            	�ɹ����ã�
���ܣ�	����RSA���ܺ�����ġ�
ע�⣺	��RSA���ܽ����У������ܻ���ܵ��ַ������ȣ�Ҫ�󲻴���N�ĳ��ȡ�
**************************************************************************************/


int Decrypt(U8*p_decrypted,U8* p_crypted,U8*p_dbyte,U8*p_Nbyte,U16 pdec_len, U16 pec_len,U16 pd_len,U16 pN_len  )
{
U8 Np[NSIZE],EC[NSIZE],DEC[NSIZE];            //NSIZEָ��ԿN���ֽڳ���
U16 dec_len=pdec_len;                         //��¼DEC,EC��d��N����ĳ���
U16 ec_len =pec_len;
U16 d_len  =pd_len;
U16 N_len  =pN_len;
if(p_decrypted==NULL||p_crypted==NULL||p_dbyte==NULL||p_Nbyte==NULL||dec_len<=0||ec_len<=0||d_len<=0||N_len<=0)
{
	printf("�����ܲ�������!\n");             //����У��
	return -1;

}
if(ec_len>NSIZE)                             //�ж������Ƿ������Ӧ������N���ֽڳ��ȣ�
{
	printf("�������ַ���������\n");
    return -2;
}
memset(Np,0,NSIZE);                          // �������
memcpy(Np,p_Nbyte,N_len);					 // ʹ��������������ΪNSIZE���ֽڣ�����ԿN�ĳ�����ͬ��
memset(EC,0,NSIZE);							 // D����Ϊָ��������չ
memcpy(EC,p_crypted,ec_len);
memset(DEC,0,NSIZE);
memcpy(DEC,p_decrypted,dec_len);
Rec_Power_Mod(DEC,EC,p_dbyte,Np,d_len);
memcpy(p_decrypted,DEC,dec_len);
return 0;
}



/*****************************************RSA���ܺ���************************************
������
U8* p_ecrypting 	 ���ܺ�������ַ�����            ���������
U8* p_original  	 �����ܵ������ַ�����            �����룩��
U8* p_ebyte     	 ������Կe          			 �����룩��
U8*p_Nbyte      	 ��Կ����N        				 �����룩��
U16 pecing_len  	 ��������p_decrypted�ĳ���		 �����룩��
U16 poriginal_len  	 ��������p_crypted�ĳ���         �����룩��
U16 pe_len           ����p_ebyte�ĳ���               �����룩��
U16 pN_len           ����p_Nbyte�ĳ���               �����룩��
����ֵ�� 
 -2            ����: ��������ָ��Ϊ�ջ����鳤�Ȳ������㣻
 -1            �������ĳ���pec_len����N�ĳ��ȣ�
  0            �ɹ����ã�
���ܣ���RSA�㷨�������ģ��������ġ�
ע�⣺��RSA���ܽ����У������ܻ���ܵ��ַ������ȣ�Ҫ�󲻴���N�ĳ��ȡ�
**************************************************************************************/

int Ecrypt(U8*p_ecrypting,U8* p_original,U8*p_ebyte,U8*p_Nbyte,U16 pecing_len, U16 poriginal_len,U16 pe_len,U16 pN_len )
{
U8 Np[NSIZE],EC[NSIZE],OR[NSIZE];
U16 ecing_len=pecing_len;
U16 original_len =poriginal_len;
U16 e_len  =pe_len;
U16 N_len  =pN_len;
if(p_ecrypting==NULL||p_original==NULL||p_ebyte==NULL||p_Nbyte==NULL||ecing_len<=0||original_len<=0||e_len<=0||N_len<=0)
{
	printf("�����ܲ�������!\n");             //����У��
	return -1;

}
if(original_len>NSIZE)                       //�жϴ����ܵ������Ƿ������Ӧ������N���ֽڳ��ȣ�
{
	printf("�������ַ���������\n");
    return -2;
}
memset(Np,0,NSIZE);                          //�������
memcpy(Np,p_Nbyte,N_len);
memset(OR,0,NSIZE);
memcpy(OR,p_original,original_len);
memset(EC,0,NSIZE);
memcpy(EC,p_ecrypting,ecing_len);
Rec_Power_Mod(EC,OR,p_ebyte,Np,e_len);
memcpy(p_ecrypting,EC,ecing_len);
return 0;
}






















