/**
 * @file my_assembler_20211448.c
 * @date 2024-04-09
 * @version 0.1.0
 *
 * @brief SIC/XE 소스코드를 object code로 변환하는 프로그램
 *
 * @details
 * SIC/XE 소스코드를 해당 머신에서 동작하도록 object code로 변환하는
 * 프로그램이다. 파일 내에서 사용되는 문자열 "00000000"에는 자신의 학번을
 * 기입한다.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 파일명의 "00000000"은 자신의 학번으로 변경할 것 */
#include "my_assembler_20211448.h"

/**
 * @brief 사용자로부터 SIC/XE 소스코드를 받아서 object code를 출력한다.
 *
 * @details
 * 사용자로부터 SIC/XE 소스코드를 받아서 object code를 출력한다. 특별한 사유가
 * 없는 한 변경하지 말 것.
 */
int main(int argc, char **argv) {
	/** SIC/XE 머신의 instruction 정보를 저장하는 테이블 */
	inst *inst_table[MAX_INST_TABLE_LENGTH];
	int inst_table_length;

	/** SIC/XE 소스코드를 저장하는 테이블 */
	char *input[MAX_INPUT_LINES];
	int input_length;

	/** 소스코드의 각 라인을 토큰 전환하여 저장하는 테이블 */
	token *tokens[MAX_INPUT_LINES];
	int tokens_length;

	/** 소스코드 내의 심볼을 저장하는 테이블 */
	symbol *symbol_table[MAX_TABLE_LENGTH];
	int symbol_table_length;

	/** 소스코드 내의 리터럴을 저장하는 테이블 */
	literal *literal_table[MAX_TABLE_LENGTH];
	int literal_table_length;

	/** 오브젝트 코드를 저장하는 변수 */
	object_code *obj_code = (object_code*)calloc(1, sizeof(object_code));
	if(obj_code==NULL){
		return -2;
	}

	int err = 0;

	if ((err = init_inst_table(inst_table, &inst_table_length,
							   "inst_table.txt")) < 0) {
		fprintf(stderr,
				"init_inst_table: 기계어 목록 초기화에 실패했습니다. "
				"(error_code: %d)\n",
				err);
		return -1;
	}

	if ((err = init_input(input, &input_length, "input.txt")) < 0) {
		fprintf(stderr,
				"init_input: 소스코드 입력에 실패했습니다. (error_code: %d)\n",
				err);
		return -1;
	}

	if ((err = assem_pass1((const inst **)inst_table, inst_table_length,
						   (const char **)input, input_length, tokens,
						   &tokens_length, symbol_table, &symbol_table_length,
						   literal_table, &literal_table_length)) < 0) {
		fprintf(stderr,
				"assem_pass1: 패스1 과정에서 실패했습니다. (error_code: %d)\n",
				err);
		return -1;
	}

	if ((err = make_symbol_table_output("output_symtab.txt",
										(const symbol **)symbol_table,
										symbol_table_length)) < 0) {
		fprintf(stderr,
				"make_symbol_table_output: 심볼테이블 파일 출력 과정에서 "
				"실패했습니다. (error_code: %d)\n",
				err);
		return -1;
	}

	if ((err = make_literal_table_output("output_littab.txt",
										 (const literal **)literal_table,
										 literal_table_length)) < 0) {
		fprintf(stderr,
				"make_literal_table_output: 리터럴테이블 파일 출력 과정에서 "
				"실패했습니다. (error_code: %d)\n",
				err);
		return -1;
	}

	if ((err = assem_pass2((const token **)tokens, tokens_length,
						   (const inst **)inst_table, inst_table_length,
						   (const symbol **)symbol_table, symbol_table_length,
						   (const literal **)literal_table,
						   literal_table_length, obj_code)) < 0) {
		fprintf(stderr,
				"assem_pass2: 패스2 과정에서 실패했습니다. (error_code: %d)\n",
				err);
		return -1;
	}

	if ((err = make_objectcode_output("output_objectcode.txt",
									  (const object_code *)obj_code)) < 0) {
		fprintf(stderr,
				"make_objectcode_output: 오브젝트코드 파일 출력 과정에서 "
				"실패했습니다. (error_code: %d)\n",
				err);
		return -1;
	}

	return 0;
}

/**
 * @brief 기계어 목록 파일(inst_table.txt)을 읽어 기계어 목록
 * 테이블(inst_table)을 생성한다.
 *
 * @param inst_table 기계어 목록 테이블의 시작 주소
 * @param inst_table_length 기계어 목록 테이블의 길이를 저장하는 변수 주소
 * @param inst_table_dir 기계어 목록 파일 경로
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 기계어 목록 파일(inst_table.txt)을 읽어 기계어 목록 테이블(inst_table)을
 * 생성한다. 기계어 목록 파일 형식은 자유롭게 구현한다. 예시는 다음과 같다.
 *    ==============================================================
 *           | 이름 | 형식 | 기계어 코드 | 오퍼랜드의 갯수 | \n |
 *    ==============================================================
 */
int init_inst_table(inst *inst_table[], int *inst_table_length,
					const char *inst_table_dir) {
	FILE *fp;
	// 읽기 권한으로 파일입출력을 시작함
	fp = fopen(inst_table_dir, "r");
	int err = 0;
	
	// 파일입출력이 정상적이지 않았을 때 error
	if(fp==NULL){
		return err = -1;
	}
	
	// 아무 입력도 받지 않은 상태에서 0으로 초기화
	*inst_table_length = 0;
	
	// inst 구조체를 이용해 input 변수 생성
	inst input;
	
	while(!feof(fp)){
		// 파일에서 데이터를 가져옴
		memset(&input, 0, sizeof(input));
		fscanf(fp, "%9s\t%d\t%hhx\t%d\n", input.str, &input.format, &input.op, &input.ops);
		
		// 현재 입력받은 instruction의 수가 최대 instruction의 수보다 많을 때 error
		if(*inst_table_length > MAX_INST_TABLE_LENGTH){
			return err = -10001;
		}
		
		// 입력받은 format이 3개 이상일 때 error
		if(input.format>99){
			return err = -10002;
		}
		// 입력받은 format이 2개일 때
		if(input.format>9){
			// 같은 타입이면 error
			if(input.format/10 == input.format%10){
				return err = -10003;
			}
			// 첫번째 형식이 1~4형식이 아니라면 error
			if(input.format/10 > 4 || input.format/10 < 1){
				return err = -10004;
			}
			// 두번째 형식이 1~4형식이 아니라면 error
			if(input.format%10 > 4 || input.format%10 < 1){
				return err = -10005;
			}
		}
		else {
			if(input.format > 4 || input.format < 1){
				return err = -10006;
			}
		}
		// 입력받은 opcode가 4의 배수가 아닐 때 error
		if(input.op % 4 != 0){
			return err = -10007;
		}
		
		// 입력받은 operand의 수가 음수일 때 또는 operand 수가 최대 operand 수를 넘어갈 때 error
		if(input.ops < 0 || MAX_OPERAND_PER_INST < input.ops){
			return err = -10008;
		}
		
		// instruction table에 입력받은 값을 저장하기 위해 동적할당
		inst_table[*inst_table_length] = (inst*)calloc(1, sizeof(inst));
		
		// 동적할당에 실패했을 때 error
		if(inst_table[*inst_table_length] == NULL){
			return err = -2;
		}
		
		// 할당한 공간에 입력받은 데이터를 복사함
		memcpy(inst_table[*inst_table_length], &input, sizeof(inst));
		// inst_table_length의 값을 1 증가 함
		*inst_table_length += 1;
		
	}
	// 파일 입출력을 종료함
	fclose(fp);

	// 정상종료 시 0을 반환함
	return err;
}

/**
 * @brief SIC/XE 소스코드 파일(input.txt)을 읽어 소스코드 테이블(input)을
 * 생성한다.
 *
 * @param input 소스코드 테이블의 시작 주소
 * @param input_length 소스코드 테이블의 길이를 저장하는 변수 주소
 * @param input_dir 소스코드 파일 경로
 * @return 오류 코드 (정상 종료 = 0)
 */
int init_input(char *input[], int *input_length, const char *input_dir) {
	FILE *fp;
	// 읽기용 변수
	char buf[100];
	memset(buf, 0, sizeof(buf));
	int len;
	// 읽기 권한으로 파일입출력을 시작함
	fp = fopen(input_dir, "r");
	int err = 0;
	
	// 파일입출력에 실패하면 error
	if(fp == NULL){
		return err = -1;
	}
	
	
	// fgets를 통해 한 줄 씩 읽고 개행포함 최대 100개 파일에 읽을 것이 없거나 에러가 발생하면 중단 함
	while(fgets(buf, 100, fp) != NULL){
		
		// 입력받은 데이터가 최대 line을 넘어가면 error
		if(MAX_TABLE_LENGTH < *input_length){
			return err = -1;
		}
		
		// 입력값의 마지막값을 '\0'으로 변경
		len = strlen(buf);
		if(buf[len-1]=='\n'){
			buf[len-1] = 0;
		}
		
		// 크기에 맞게 동적할당
		input[*input_length] = (char*)calloc(1, len);
		
		// 동적할당에 실패하면 error
		if(input[*input_length]==NULL){
			return err = -1;
		}
		
		// buf값을 input값에 저장
		strncpy(input[*input_length], buf, strlen(buf));
		
		// input_length 값을 1 증가
		*input_length += 1;
		memset(buf, 0, sizeof(buf));
	}

	// 파일 입출력을 종료
	fclose(fp);
	
	// 오류가 없다면 0을 반환
	return err;
}

/**
 * @brief 어셈블리 코드을 위한 패스 1 과정을 수행한다.
 *
 * @param inst_table 기계어 목록 테이블의 주소
 * @param inst_table_length 기계어 목록 테이블의 길이
 * @param input 소스코드 테이블의 주소
 * @param input_length 소스코드 테이블의 길이
 * @param tokens 토큰 테이블의 시작 주소
 * @param tokens_length 토큰 테이블의 길이를 저장하는 변수 주소
 * @param symbol_table 심볼 테이블의 시작 주소
 * @param symbol_table_length 심볼 테이블의 길이를 저장하는 변수 주소
 * @param literal_table 리터럴 테이블의 시작 주소
 * @param literal_table_length 리터럴 테이블의 길이를 저장하는 변수 주소
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 어셈블리 코드를 위한 패스1 과정을 수행하는 함수이다. 패스 1에서는 프로그램
 * 소스를 스캔하여 해당하는 토큰 단위로 분리하여 프로그램 라인별 토큰 테이블을
 * 생성한다. 토큰 테이블은 token_parsing 함수를 호출하여 설정하여야 한다. 또한,
 * assem_pass2 과정에서 사용하기 위한 심볼 테이블 및 리터럴 테이블을 생성한다.
 */
int assem_pass1(const inst *inst_table[], int inst_table_length,
				const char *input[], int input_length, token *tokens[],
				int *tokens_length, symbol *symbol_table[],
				int *symbol_table_length, literal *literal_table[],
				int *literal_table_length) {
	// 길이를 0으로 초기화
	*tokens_length = 0;
	*symbol_table_length = 0;
	*literal_table_length = 0;
	
	// Pass 1 과정에서 필요한 임시변수들을 선언
	inst tmp_inst;
	token tmp_token;
	symbol tmp_symbol;
	literal tmp_literal;
	char tmp[10];
	char tmp_base[10];
	int inst_index = 0;
	
	// opcode의 format을 저장하는 변수
	int format1=0, format2=0;
	
	// Location Counter를 정의
	int location_counter = 0;
	
	for(int i=0;i<input_length;i++){
		tokens[*tokens_length] = (token*)calloc(1, sizeof(token));
		if(tokens[*tokens_length]==NULL)return -2;
		if(token_parsing(input[i], tokens[(*tokens_length)++], inst_table, inst_table_length)<0){
			return -1;
		}
		
		
	}
	
	for(int i=0;i<*tokens_length;i++){
		// Pass 1과정을 진행하기 위한 정보들을 수집
		memset(&tmp_token, 0, sizeof(tmp_token));
		tmp_token = *tokens[i];
		if(tmp_token.operator!=NULL){
			inst_index = search_opcode(tmp_token.operator, inst_table, inst_table_length);
			if(inst_index!=-1){
				memset(&tmp_inst, 0, sizeof(tmp_inst));
				tmp_inst = *inst_table[inst_index];
			}
		}
			
		
		// 라인 주석인 경우 Pass 1과정에서 필요없으니 건너뜀
		if(tmp_token.label==NULL && tmp_token.operator==NULL){
			continue;
		}
		
		// Location Counter를 START의 operand[0]로 지정
		if(!strcmp(tmp_token.operator, "START")){
			location_counter = atoi(tmp_token.operand[0]);
			memset(tmp_base, 0, sizeof(tmp_base));
			strncpy(tmp_base, tmp_token.label, strlen(tmp_token.label));
		}
		// CSECT를 만났을 경우
		if(!strcmp(tmp_token.operator, "CSECT")){
			memset(tmp_base, 0, sizeof(tmp_base));
			strncpy(tmp_base, tmp_token.label, strlen(tmp_token.label));
			location_counter = 0;
		}
		
		// Label이 존재하는 경우 SYMTAB에 저장
		if(tmp_token.label!=NULL){
			memset(&tmp_symbol, 0, sizeof(tmp_symbol));
			strncpy(tmp_symbol.name, tmp_token.label, strlen(tmp_token.label));
			if(!strcmp(tmp_token.operator, "EQU")){
				tmp_symbol.addr = -1;
			}
			else tmp_symbol.addr = location_counter;
			strncpy(tmp_symbol.base, tmp_base, strlen(tmp_base));
			symbol_table[*symbol_table_length] = (symbol*)calloc(1, sizeof(symbol));
			if(symbol_table[*symbol_table_length]==NULL)return -2;
			memcpy(symbol_table[(*symbol_table_length)++], &tmp_symbol, sizeof(tmp_symbol));
		}
		
		// Location Counter를 증가시키는 로직
		// 1~4형식을 사용하는 instruction인 경우
		if(tmp_token.operator!=NULL && inst_index != -1){
			// format1과 2를 가져온다.
			// 1, 2형식인 경우 format1 = 0, format2는 1 또는 2이다.
			// 3, 4형식인 경우 format1 = 3, format2는 4이다.
			format1 = tmp_inst.format/10;
			format2 = tmp_inst.format%10;
			
			if(format2==1)location_counter += 1;
			else if(format2==2)location_counter += 2;
			// 4형식을 지원하는 명령어가 실제로 4형식인지 확인
			else if(format2==4 && *tmp_token.operator == '+'){
				location_counter += 4;
				// nixbpe 중 e 비트를 1로 채움
				tmp_token.nixbpe |= 49;
			}
			else if(format1==3){
				location_counter += 3;
				// ni 비틀를 1로 채움 immediate는 나중에 고려
				tmp_token.nixbpe |= 48;
				// pc 비트를 1로 채움
				tmp_token.nixbpe |= 2;
			}
		}
		// operator과 "WORD", "RESW", "RESB", "BYTE" 인 경우
		else if(!strcmp(tmp_token.operator, "WORD")){
			location_counter += 3;
		}
		else if(!strcmp(tmp_token.operator, "RESW")){
			location_counter += 3 * atoi(tmp_token.operand[0]);
		}
		else if(!strcmp(tmp_token.operator, "RESB")){
			location_counter += atoi(tmp_token.operand[0]);
		}
		else if(!strcmp(tmp_token.operator, "BYTE")){
			// 'X' 또는 'C'와 따옴표 2개의 길이를 뺀 실제 operand의 길이
			if(*(tmp_token.operand[0])=='X'){
				location_counter += (strlen(tmp_token.operand[0]) - 3)/2;
			}
			else {
				location_counter += strlen(tmp_token.operand[0]) - 3;
			}
		}
		// EQU인 경우
		else if(!strcmp(tmp_token.operator, "EQU")){
			if(*tmp_token.operand[0] == '*'){
				symbol_table[*symbol_table_length - 1]->addr = location_counter;
			}
			// 사칙연산을 사용하는 EQU
			else {
				int opcode_index=0;
				for(int k=0;k<strlen(tmp_token.operand[0]);k++){
					if(tmp_token.operand[0][k]=='+' ||
					   tmp_token.operand[0][k]=='-' ||
					   tmp_token.operand[0][k]=='/' ||
					   tmp_token.operand[0][k]=='*'){
						opcode_index=k;
						break;
					}
				}
				int tmp_addr = 0;
				// 왼쪽 심볼
				memset(tmp, 0, sizeof(tmp));
				strncpy(tmp, tmp_token.operand[0], opcode_index);
				for(int k=0;k<*symbol_table_length-1;k++){
					if(!strcmp(symbol_table[k]->name,tmp)){
						tmp_addr += symbol_table[k]->addr;
					}
				}
				// 오른쪽 심볼
				int flag = 0;
				memset(tmp, 0, sizeof(tmp));
				strncpy(tmp, tmp_token.operand[0] + opcode_index + 1,
						strlen(tmp_token.operand[0]) - (opcode_index + 1));
				for(int k=0;k<*symbol_table_length-1;k++){
					if(!strcmp(symbol_table[k]->name,tmp)){
						switch(tmp_token.operand[0][opcode_index]){
							case('+') : tmp_addr += symbol_table[k]->addr;break;
							case('-'): tmp_addr -= symbol_table[k]->addr;break;
							case('/') : tmp_addr /= symbol_table[k]->addr;break;
							case('*') : tmp_addr *= symbol_table[k]->addr;break;
							default: return -1;
						}
						flag = 1;
					}
				}
				// 오른쪽이 심볼이 아닌경우
				if(!flag){
					switch(tmp_token.operand[0][opcode_index]){
						case('+') : tmp_addr += atoi(tmp);break;
						case('-'): tmp_addr -= atoi(tmp);break;
						case('/') : tmp_addr /= atoi(tmp);break;
						case('*') : tmp_addr *= atoi(tmp);break;
						default: return -1;
					}
				}
				// 구한 값을 넣어줌
				tmp_symbol.addr=tmp_addr;
				strncpy(tmp_symbol.base, tmp_symbol.name, strlen(tmp_symbol.name));
				memcpy(symbol_table[*symbol_table_length - 1], &tmp_symbol, sizeof(tmp_symbol));
			}
		}
		
		
		
		// LTORG를 만나거나 END를 만났을 경우
		if(!strcmp(tmp_token.operator, "LTORG") || !strcmp(tmp_token.operator, "END")){
			for(int k=0;k<*literal_table_length;k++){
				// 할당되어 있지 않은 리터럴들을 할당
				if(literal_table[k]->addr==-1){
					literal_table[k]->addr= location_counter;
					location_counter += literal_table[k]->size;
				}
			}
		}
		
		// operand가 '='로 시작하여 Literal을 의미하는 경우
		if(tmp_token.operand[0]==NULL)continue;
		if(*tmp_token.operand[0]=='='){
			memset(&tmp_literal, 0, sizeof(tmp_literal));
			// 이미 리터럴이 있는지 확인
			int flag = 0;
			for(int k=0;k<*literal_table_length;k++){
				if(!strcmp(literal_table[k]->literal, tmp_token.operand[0]))
				   flag = 1;
			}
			if(!flag){
				strncpy(tmp_literal.literal, tmp_token.operand[0], strlen(tmp_token.operand[0]));
				strncpy(tmp_literal.base, tmp_base, strlen(tmp_base));
				tmp_literal.addr = -1;
				tmp_literal.size = strlen(tmp_literal.literal) - 4;
				literal_table[*literal_table_length] = (literal*)calloc(1, sizeof(literal));
				if(literal_table[*literal_table_length]==NULL){
					return -2;
				}
				memcpy(literal_table[(*literal_table_length)++], &tmp_literal, sizeof(tmp_literal));
			}
		}
		
		// nixbpe를 설정
		for(int k=0;k<MAX_OPERAND_PER_INST && tmp_token.operand[k]!=NULL;k++){
			// base 사용하는지 확인
			if(!strcmp(tmp_token.operand[k], "base")){
				tmp_token.nixbpe &= 48;
				tmp_token.nixbpe |= 4;
			}
			// X 레지스터를 사용하는지 확인
			if(!strcmp(tmp_token.operand[k], "X")){
				tmp_token.nixbpe |= 8;
			}
			// immediate인지 확인
			if(*tmp_token.operand[k] == '#'){
				tmp_token.nixbpe &= 16;
			}
			if(*tmp_token.operand[k] == '@'){
				tmp_token.nixbpe &= 32;
				tmp_token.nixbpe |= 2;
			}
		}
		
		// 갱신한 nixbpe값을 저장
		memcpy(tokens[i], &tmp_token, sizeof(tmp_token));
	}
	
	
	return 0;
}

/**
 * @brief 한 줄의 소스코드를 파싱하여 토큰에 저장한다.
 *
 * @param input 파싱할 소스코드 문자열
 * @param tok 결과를 저장할 토큰 구조체 주소
 * @param inst_table 기계어 목록 테이블의 주소
 * @param inst_table_length 기계어 목록 테이블의 길이
 * @return 오류 코드 (정상 종료 = 0)
 */
int token_parsing(const char *input, token *tok, const inst *inst_table[],
				  int inst_table_length) {
	// 주석, Label, operator 등을 입력 받을 임시 문자열 생성
	char tmp[100];
	// 문자열의 끝을 저장
	char *end = input + strlen(input);
	// operand를 파싱하는 과정에서 사용될 변수들을 선언
	char *operand;
	unsigned int operand_length, operands;
	
	// token 매개변수를 초기화
	tok->label = NULL;
	tok->operator = NULL;
	tok->nixbpe = 0;
	tok->operand[0] = tok->operand[1] = tok->operand[2] = NULL;
	tok->comment = NULL;
	
	// 문자열을 다 읽었는지 확인 하는 과정, 매 케이스마다 계속 등장함
	if(input >= end) return 0;
	// 해당 라인이 주석이라면 입력을 받고 리턴
	if(*input=='.'){
		sscanf(input+1, "%99[^\n]", tmp);
		tok->comment = calloc(1, strlen(tmp));
		if(tok->comment==NULL)return -2;
		strncpy(tok->comment, tmp, strlen(tmp));
		// 주석라인은 주석만 존재하므로 다 읽고 리턴함
		return 0;
	}
	
	if(input >= end) return 0;
	// 현재 input위치에 '\t'가 아니라면 Label이 존재하므로 Label일 때 읽음
	if(*input!='\t'){
		// 문자열을 읽고 읽은만큼 포인터를 이동함
		sscanf(input, "%s", tmp);
		input += strlen(tmp);
		tok->label = calloc(1, strlen(tmp));
		if(tok->label==NULL)return -2;
		strncpy(tok->label, tmp, strlen(tmp));
	}
	// '\t'를 건너뜀
	input += 1;
	
	if(input >= end) return 0;
	// 현재 input위치에 '\t'가 아니라면 operator이 존재하므로 operator일 때 읽음
	if(*input!='\t'){
		// 문자열을 읽고 읽은만큼 포인터를 이동함
		sscanf(input, "%s", tmp);
		input += strlen(tmp);
		tok->operator = calloc(1, strlen(tmp));
		if(tok->operator==NULL)return -2;
		strncpy(tok->operator, tmp, strlen(tmp));
	}
	// '\t'를 건너뜀
	input += 1;
	
	if(input >= end) return 0;
	// 현재 input위치에 '\t'가 아니라면 operand이 존재하므로 operand일 때 읽음
	if(*input!='\t'){
		// 문자열을 읽고 읽은만큼 포인터를 이동함
		sscanf(input, "%s", tmp);
		input += strlen(tmp);
		// operand_length는 0으로 operand는 tmp의 시작주소로 지정
		operand_length = operands = 0;
		operand = tmp;
		// operand의 포인터를 이동시킬 것이기 때문에 반복문 종료조건을 다음과 같이 설정
		while(input >= operand){
			// 최대 오퍼랜드 개수를 넘어가면 에러를 반환
			// 이미 3개를 입력받은 상황에서 반복문을 더 수행하는 것은 이상함으로 에러를 반환
			if(operands>=MAX_OPERAND_PER_INST){
				return -1;
			}
			
			if(operand[operand_length]==','){
				tok->operand[operands] = calloc(1, operand_length);
				if(tok->operand[operands]==NULL)return -2;
				strncpy(tok->operand[operands++], operand, operand_length);
				// ','로 구분되어 있다면 다음 오퍼랜드가 있다는 것을 의미해서 operand 주소를 다음 오퍼랜드 시작주소로 넘김
				operand += operand_length + 1;
				continue;
			}
			
			if(operand[operand_length]=='\0'){
				tok->operand[operands] = calloc(1, operand_length);
				if(tok->operand[operands]==NULL)return -2;
				strncpy(tok->operand[operands++], operand, operand_length);
				// 문자열의 끝을 만났다는 것은 마지막 오퍼랜드라는 것을 의미하기 때문에 반복물을 종료함
				break;
			}
			// 계속 증가함
			operand_length++;
		}
		
	}
	// '\t'를 건너뜀
	input += 1;
	if(input >= end) return 0;
	// 지금까지 남은 문자가 있다면 그것은 모두 주석으로 간주
	sscanf(input, "%99[^\n]", tmp);
	tok->comment = calloc(1, strlen(tmp));
	if(tok->comment==NULL)return -2;
	strncpy(tok->comment, tmp, strlen(tmp));
	// 항상 마지막은 주석이므로 주석을 만났기 때문에 0을 반환 함
	return 0;
}

/**
 * @brief 기계어 목록 테이블에서 특정 기계어를 검색하여, 해당 기계에가 위치한
 * 인덱스를 반환한다.
 *
 * @param str 검색할 기계어 문자열
 * @param inst_table 기계어 목록 테이블 주소
 * @param inst_table_length 기계어 목록 테이블의 길이
 * @return 기계어의 인덱스 (해당 기계어가 없는 경우 -1)
 *
 * @details
 * 기계어 목록 테이블에서 특정 기계어를 검색하여, 해당 기계에가 위치한 인덱스를
 * 반환한다. '+JSUB'와 같은 문자열에 대한 처리는 자유롭게 처리한다.
 */
int search_opcode(const char *str, const inst *inst_table[],
				  int inst_table_length) {
	// str이 4형식일 때
	if(*str=='+')str += 1;
	// 최대 instruction 수는 256개 이고 최대 라인수는 5000이기 떄문에 선형탐색을 채택
	// 256 * 5000 연산은 0.1초 미만의 시간이 걸림
	
	for(int i=0;i<inst_table_length;i++){
		// strcmp는 두 문자열이 같으면 0을 반환
		if(!strcmp(str, inst_table[i]->str)){
			// 같은 문자열이라면 op코드를 반환
			return i;
		}
	}

	return -1;
}

/**
 * @brief 어셈블리 코드을 위한 패스 2 과정을 수행한다.
 *
 * @param tokens 토큰 테이블 주소
 * @param tokens_length 토큰 테이블 길이
 * @param inst_table 기계어 목록 테이블 주소
 * @param inst_table_length 기계어 목록 테이블 길이
 * @param symbol_table 심볼 테이블 주소
 * @param symbol_table_length 심볼 테이블 길이
 * @param literal_table 리터럴 테이블 주소
 * @param literal_table_length 리터럴 테이블 길이
 * @param obj_code 오브젝트 코드에 대한 정보를 저장하는 구조체 주소
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행한다. 패스 2의
 * 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
 */
int assem_pass2(const token *tokens[], int tokens_length,
				const inst *inst_table[], int inst_table_length,
				const symbol *symbol_table[], int symbol_table_length,
				const literal *literal_table[], int literal_table_length,
				object_code *obj_code) {
	
	modification_record *mod_red = (modification_record*)calloc(1, sizeof(modification_record));
	if(mod_red==NULL)return -2;
	modification_record *now_red = mod_red;
	
	// Pass 2 과정에서 필요한 임시변수들을 선언
	inst tmp_inst;
	token tmp_token;
	char pro_name[10];
	int pro_start = 0;
	char tmp_hex[10];
	char tmp_base[10];
	char tmp_ref[MAX_OPERAND_PER_INST][10];
	int ref_cnt = 0;
	int inst_index = 0;
	int ltorg_flag = 0;
	
	// 왼쪽과 오른쪽 문자열의 길이를 저장하는 변수
	int left_str=0, right_str=0;
	// Location Counter를 정의
	int location_counter = 0;
	int total = 0;
	int pos = 0;
	
	// 프로그램 크기를 저장
	int pro_size[10];
	int pro_cnt = 0;
	memset(pro_size, 0, sizeof(pro_size));
	
	// 현재 포인터를 지정
	object_code *now = obj_code;
	memset(now->line, 0, sizeof(now->line));
	
	for(int i=0;i<tokens_length;i++){
		// 필요한 정보들을 가져옴
		memset(&tmp_token, 0, sizeof(tmp_token));
		tmp_token = *tokens[i];
		if(tmp_token.operator!=NULL){
			inst_index = search_opcode(tmp_token.operator, inst_table, inst_table_length);
			if(inst_index!=-1){
				memset(&tmp_inst, 0, sizeof(tmp_inst));
				tmp_inst = *inst_table[inst_index];
			}
		}
		
		// 주석 라인을 건너뜀
		if(tmp_token.operator==NULL){
			continue;
		}
		
		// operator가 "START"인 경우
		else if(!strcmp(tmp_token.operator, "START")){
			memset(pro_name, 0, sizeof(pro_name));
			memset(tmp_base, 0, sizeof(tmp_base));
			strncpy(pro_name, tmp_token.label, strlen(tmp_token.label));
			strncpy(tmp_base, tmp_token.label, strlen(tmp_token.label));
			// "H"문자열 추가
			strcat(now->line, "H");
			left_str = strlen(now->line);
			right_str = strlen(tmp_token.label);
			// 두 문자열의 합이 99이상이면 에러 '\t'를 붙일 것이기 때문
			if(left_str + right_str >= 99)return -1;
			// Header를 정의
			strcat(now->line, tmp_token.label);
			strcat(now->line, "\t");
			
			// 시작주소를 체크
			memset(tmp_hex, 0, sizeof(tmp_hex));
			sprintf(tmp_hex, "%06X", atoi(tmp_token.operand[0]));
			pro_start = atoi(tmp_token.operand[0]);
			pos = atoi(tmp_token.operand[0]);
			left_str = strlen(now->line);
			right_str = strlen(tmp_hex);
			// 두 문자열의 합이 100이상이면 에러
			if(left_str + right_str >= 100)return -1;
			// Header를 정의
			strcat(now->line, tmp_hex);
			
			// 다음 포인터를 지정
			now->next = (object_code*)calloc(1, sizeof(object_code));
			if(now->next==NULL){
				return -2;
			}
			now = now->next;
			continue;
		}
		
		// operator가 "EXTDEF"인 경우
		else if(!strcmp(tmp_token.operator, "EXTDEF")){
			strcat(now->line, "D");
			for(int j=0;j<MAX_OPERAND_PER_INST && tmp_token.operand[j]!=NULL;j++){
				left_str = strlen(now->line);
				right_str = strlen(tmp_token.operand[j]);
				if(left_str + right_str >= 100)return -1;
				strcat(now->line, tmp_token.operand[j]);
				
				for(int k=0;k<symbol_table_length;k++){
					if(!strcmp(tmp_token.operand[j], symbol_table[k]->name)){
						memset(tmp_hex, 0, sizeof(tmp_hex));
						sprintf(tmp_hex, "%06X", symbol_table[k]->addr);
						left_str = strlen(now->line);
						right_str = strlen(tmp_hex);
						if(left_str + right_str >= 100)return -1;
						strcat(now->line, tmp_hex);
					}
				}
			}
			// 다음 포인터를 지정
			now->next = (object_code*)calloc(1, sizeof(object_code));
			if(now->next==NULL){
				return -2;
			}
			now = now->next;
			continue;
		}
		
		// operator가 "EXTREF"인 경우
		else if(!strcmp(tmp_token.operator, "EXTREF")){
			ref_cnt = 0;
			for(int j=0;j<MAX_OPERAND_PER_INST;j++){
				memset(tmp_ref[j], 0, sizeof(tmp_ref[j]));
			}
			strcat(now->line, "R");
			for(int j=0;j<MAX_OPERAND_PER_INST && tmp_token.operand[j]!=NULL;j++){
				left_str = strlen(now->line);
				right_str = 6;
				if(left_str + right_str >= 100)return -1;
				sprintf(tmp_hex, "%-6s", tmp_token.operand[j]);
				strcat(now->line, tmp_hex);
				
				// 해당 루틴에서 사용할 레퍼런스들을 저장
				strncpy(tmp_ref[j], tmp_token.operand[j], strlen(tmp_token.operand[j]));
				ref_cnt++;
			}
			// 다음 포인터를 지정
			now->next = (object_code*)calloc(1, sizeof(object_code));
			if(now->next==NULL){
				return -2;
			}
			now = now->next;
			continue;
		}
		
		// operator가 "CSECT"인 경우
		else if(!strcmp(tmp_token.operator, "CSECT")){
			// line이 있는 경우만 새로운 줄을 생성
			if(strlen(now->line)>0){
				char tmp[100];
				memset(tmp, 0, sizeof(tmp));
				int now_pos = strlen(now->line)/2;
				sprintf(tmp, "T%06X%02X", pos, now_pos);
				pos += now_pos;
				strcat(tmp, now->line);
				memset(now->line, 0, sizeof(now->line));
				strncpy(now->line, tmp, strlen(tmp));
				
				
				now->next = (object_code*)calloc(1, sizeof(object_code));
				if(now->next==NULL){
					return -2;
				}
				now = now->next;
			}
			
			if(!ltorg_flag){
				
				for(int k=0;k<literal_table_length;k++){
					// 리터럴 확인
					if(!strcmp(literal_table[k]->base, tmp_base)){
						memset(tmp_hex, 0, sizeof(tmp_hex));
						// 리터럴 타입 확인 후 적절하게 tmp_hex에 넣어줌
						if(*(literal_table[k]->literal + 1)=='C'){
							location_counter += literal_table[k]->size;
							char h[4];
							for(int j=3;j<3+literal_table[k]->size;j++){
								memset(h, 0, sizeof(h));
								sprintf(h, "%02X", literal_table[k]->literal[j]);
								strcat(tmp_hex, h);
							}
							
						}
						else if(*(literal_table[k]->literal + 1)=='X'){
							location_counter += literal_table[k]->size/2;
							strncpy(tmp_hex, literal_table[k]->literal+3, literal_table[k]->size);
						}
						left_str = strlen(now->line);
						right_str = strlen(tmp_hex);
						if(left_str + right_str >= 100)return -1;
						strcat(now->line, tmp_hex);
					}
				}
				// line이 있는 경우만 새로운 줄을 생성
				if(strlen(now->line)>0){
					char tmp[100];
					memset(tmp, 0, sizeof(tmp));
					int now_pos = strlen(now->line)/2;
					sprintf(tmp, "T%06X%02X", location_counter - now_pos, now_pos);
					strcat(tmp, now->line);
					memset(now->line, 0, sizeof(now->line));
					strncpy(now->line, tmp, strlen(tmp));
					now->next = (object_code*)calloc(1, sizeof(object_code));
					if(now->next==NULL){
						return -2;
					}
					now = now->next;
				}
			}
			
			
			
			while(mod_red->next != NULL){
				// 헤더
				strcat(now->line, "M");
				// 주소
				memset(tmp_hex, 0, sizeof(tmp_hex));
				sprintf(tmp_hex, "%06X", mod_red->addr);
				strcat(now->line, tmp_hex);
				// 위치
				memset(tmp_hex, 0, sizeof(tmp_hex));
				sprintf(tmp_hex, "%02X", mod_red->pos);
				strcat(now->line, tmp_hex);
				// op, name
				memset(tmp_hex, 0, sizeof(tmp_hex));
				sprintf(tmp_hex, "%c%s", mod_red->op, mod_red->name);
				strcat(now->line, tmp_hex);
				
				mod_red = mod_red->next;
				
				// line이 있는 경우만 새로운 줄을 생성
				if(strlen(now->line)>0){
					now->next = (object_code*)calloc(1, sizeof(object_code));
					if(now->next==NULL){
						return -2;
					}
					now = now->next;
				}
				
			}
			
			strcat(now->line, "E");
			if(!strcmp(pro_name, tmp_base)){
				memset(tmp_hex, 0, sizeof(tmp_hex));
				sprintf(tmp_hex, "%06X", pro_start);
				strcat(now->line, tmp_hex);
			}
			now->next = (object_code*)calloc(1, sizeof(object_code));
			if(now->next==NULL){
				return -2;
			}
			now = now->next;
			
			
			
			
			memset(tmp_base, 0, sizeof(tmp_base));
			strncpy(tmp_base, tmp_token.label, strlen(tmp_token.label));
			// "H"문자열 추가
			strcat(now->line, "H");
			left_str = strlen(now->line);
			right_str = strlen(tmp_token.label);
			// 두 문자열의 합이 99이상이면 에러 '\t'를 붙일 것이기 때문
			if(left_str + right_str >= 99)return -1;
			// Header를 정의
			strcat(now->line, tmp_token.label);
			strcat(now->line, "\t");
			
			// 시작주소 붙이기
			memset(tmp_hex, 0, sizeof(tmp_hex));
			sprintf(tmp_hex, "000000");
			left_str = strlen(now->line);
			right_str = strlen(tmp_hex);
			// 두 문자열의 합이 100이상이면 에러
			if(left_str + right_str >= 100)return -1;
			// Header를 정의
			strcat(now->line, tmp_hex);
			
			
			total += location_counter;
			pro_size[pro_cnt++] = location_counter;
			location_counter = 0;
			pos = 0;
			
			// 다음 포인터를 지정
			now->next = (object_code*)calloc(1, sizeof(object_code));
			if(now->next==NULL){
				return -2;
			}
			now = now->next;
			ltorg_flag = 0;
			continue;
		}
		
		// operator가 "END"인 경우
		else if(!strcmp(tmp_token.operator, "END")){
			if(!ltorg_flag){
				
				for(int k=0;k<literal_table_length;k++){
					
					// 리터럴 확인
					if(!strcmp(literal_table[k]->base, tmp_base)){
						memset(tmp_hex, 0, sizeof(tmp_hex));
						// 리터럴 타입 확인 후 적절하게 tmp_hex에 넣어줌
						if(*(literal_table[k]->literal + 1)=='C'){
							location_counter += literal_table[k]->size;
							char h[4];
							for(int j=3;j<3+literal_table[k]->size;j++){
								memset(h, 0, sizeof(h));
								sprintf(h, "%02X", literal_table[k]->literal[j]);
								strcat(tmp_hex, h);
							}
							
						}
						else if(*(literal_table[k]->literal + 1)=='X'){
							location_counter += literal_table[k]->size/2;
							strncpy(tmp_hex, literal_table[k]->literal+3, literal_table[k]->size);
						}
						left_str = strlen(now->line);
						right_str = strlen(tmp_hex);
						if(left_str + right_str >= 100)return -1;
						strcat(now->line, tmp_hex);
					}
				}
				// line이 있는 경우만 새로운 줄을 생성
				if(strlen(now->line)>0){
					char tmp[100];
					memset(tmp, 0, sizeof(tmp));
					int now_pos = strlen(now->line)/2;
					sprintf(tmp, "T%06X%02X", pos, now_pos);
					pos += now_pos;
					strcat(tmp, now->line);
					memset(now->line, 0, sizeof(now->line));
					strncpy(now->line, tmp, strlen(tmp));
					
					now->next = (object_code*)calloc(1, sizeof(object_code));
					if(now->next==NULL){
						return -2;
					}
					now = now->next;
				}
				ltorg_flag = 0;
			}
			
			while(mod_red->next != NULL){
				// 헤더
				strcat(now->line, "M");
				// 주소
				memset(tmp_hex, 0, sizeof(tmp_hex));
				sprintf(tmp_hex, "%06X", mod_red->addr);
				strcat(now->line, tmp_hex);
				// 위치
				memset(tmp_hex, 0, sizeof(tmp_hex));
				sprintf(tmp_hex, "%02X", mod_red->pos);
				strcat(now->line, tmp_hex);
				// op, name
				memset(tmp_hex, 0, sizeof(tmp_hex));
				sprintf(tmp_hex, "%c%s", mod_red->op, mod_red->name);
				strcat(now->line, tmp_hex);
				
				mod_red = mod_red->next;
				
				// line이 있는 경우만 새로운 줄을 생성
				if(strlen(now->line)>0){
					now->next = (object_code*)calloc(1, sizeof(object_code));
					if(now->next==NULL){
						return -2;
					}
					now = now->next;
				}
				
			}
			
			
			
			total += location_counter;
			pro_size[pro_cnt++] = location_counter;
			location_counter = 0;
			
			
			if(strlen(now->line)>0){
				now->next = (object_code*)calloc(1, sizeof(object_code));
				if(now->next==NULL){
					return -2;
				}
				now = now->next;
			}
			
			// "E" 추가
			strcat(now->line, "E");
			continue;
		}
		
		// operator가 "LTORG"인 경우
		else if(!strcmp(tmp_token.operator, "LTORG")){
			ltorg_flag = 1;
			// line이 있는 경우만 새로운 줄을 생성
			if(strlen(now->line)>0){
				char tmp[100];
				memset(tmp, 0, sizeof(tmp));
				int now_pos = strlen(now->line)/2;
				sprintf(tmp, "T%06X%02X", pos, now_pos);
				pos += now_pos;
				strcat(tmp, now->line);
				memset(now->line, 0, sizeof(now->line));
				strncpy(now->line, tmp, strlen(tmp));
				
				now->next = (object_code*)calloc(1, sizeof(object_code));
				if(now->next==NULL){
					return -2;
				}
				now = now->next;
			}
			
			for(int k=0;k<literal_table_length;k++){
				// 리터럴 확인
				if(!strcmp(literal_table[k]->base, tmp_base)){
					memset(tmp_hex, 0, sizeof(tmp_hex));
					// 리터럴 타입 확인 후 적절하게 tmp_hex에 넣어줌
					if(*(literal_table[k]->literal + 1)=='C'){
						location_counter += literal_table[k]->size;
						char h[4];
						for(int j=3;j<3+literal_table[k]->size;j++){
							memset(h, 0, sizeof(h));
							sprintf(h, "%02X", literal_table[k]->literal[j]);
							strcat(tmp_hex, h);
						}
						
					}
					else if(*(literal_table[k]->literal + 1)=='X'){
						location_counter += literal_table[k]->size/2;
						strncpy(tmp_hex, literal_table[k]->literal+3, literal_table[k]->size);
					}
					
					left_str = strlen(now->line);
					right_str = strlen(tmp_hex);
					if(left_str + right_str >= 100)return -1;
					strcat(now->line, tmp_hex);
				}
			}
			// line이 있는 경우만 새로운 줄을 생성
			if(strlen(now->line)>0){
				char tmp[100];
				memset(tmp, 0, sizeof(tmp));
				int now_pos = strlen(now->line)/2;
				sprintf(tmp, "T%06X%02X", location_counter - now_pos, now_pos);
				strcat(tmp, now->line);
				memset(now->line, 0, sizeof(now->line));
				strncpy(now->line, tmp, strlen(tmp));
				
				now->next = (object_code*)calloc(1, sizeof(object_code));
				if(now->next==NULL){
					return -2;
				}
				now = now->next;
			}
			continue;
		}
		
		else if(!strcmp(tmp_token.operator, "EQU")){
			continue;
		}
		// 본문 구간
		else {
			int next_flag = 0;
			// operator과 "WORD", "RESW", "RESB", "BYTE" 인 경우
			if(!strcmp(tmp_token.operator, "WORD")){
				location_counter += 3;
				memset(tmp_hex, 0, sizeof(tmp_hex));
				sprintf(tmp_hex, "000000");
				int op = 0;
				for(int k=0;tmp_token.operand[0][k]!='\0';k++){
					if(tmp_token.operand[0][k] == '+')op=k;
					else if(tmp_token.operand[0][k] == '-')op=k;
					else if(tmp_token.operand[0][k] == '*')op=k;
					else if(tmp_token.operand[0][k] == '/')op=k;
				}
				char tmp[10];
				memset(tmp, 0, sizeof(tmp));
				strncpy(tmp, tmp_token.operand[0], op);
				for(int k=0;k<ref_cnt;k++){
					if(!strcmp(tmp_ref[k], tmp)){
						strncpy(now_red->name, tmp_ref[k], strlen(tmp_ref[k]));
						strncpy(now_red->base, tmp_base, strlen(tmp_base));
						
						if(tmp_inst.format==34){
							now_red->pos = 5;
							now_red->addr = location_counter + 1;
						}
						else {
							now_red->pos = 6;
							now_red->addr = location_counter;
						}
						now_red->op = '+';
						
						now_red->next = (modification_record*)calloc(1, sizeof(modification_record));
						if(now_red->next==NULL)return -2;
						now_red = now_red->next;
					}
				}
				memset(tmp, 0, sizeof(tmp));
				strncpy(tmp, tmp_token.operand[0]+op+1, strlen(tmp_token.operand[0])-op-1);
				for(int k=0;k<ref_cnt;k++){
					if(!strcmp(tmp_ref[k], tmp)){
						strncpy(now_red->name, tmp_ref[k], strlen(tmp_ref[k]));
						strncpy(now_red->base, tmp_base, strlen(tmp_base));
						
						if(tmp_inst.format==34){
							now_red->pos = 5;
							now_red->addr = location_counter + 1;
						}
						else {
							now_red->pos = 6;
							now_red->addr = location_counter;
						}
						now_red->op = tmp_token.operand[0][op];
						
						now_red->next = (modification_record*)calloc(1, sizeof(modification_record));
						if(now_red->next==NULL)return -2;
						now_red = now_red->next;
					}
				}
			}
			else if(!strcmp(tmp_token.operator, "RESW")){
				location_counter += 3 * atoi(tmp_token.operand[0]);
				next_flag = 1;
			}
			else if(!strcmp(tmp_token.operator, "RESB")){
				location_counter += atoi(tmp_token.operand[0]);
				next_flag = 1;
			}
			else if(!strcmp(tmp_token.operator, "BYTE")){
				memset(tmp_hex, 0, sizeof(tmp_hex));
				// 'X' 또는 'C'와 따옴표 2개의 길이를 뺀 실제 operand의 길이
				if(*(tmp_token.operand[0])=='X'){
					location_counter += (strlen(tmp_token.operand[0]) - 3)/2;
					strncpy(tmp_hex, tmp_token.operand[0] + 2, strlen(tmp_token.operand[0]) - 3);
				}
				else if(*(tmp_token.operand[0])=='C'){
					location_counter += strlen(tmp_token.operand[0]) - 3;
					next_flag = 1;
				}
				
			}
			else if(!strcmp(tmp_token.operator, "RSUB")){
				location_counter += 3;
				memset(tmp_hex, 0, sizeof(tmp_hex));
				sprintf(tmp_hex, "%06X", 0x4F0000);
			}
			if(next_flag)continue;
			// n, i 비트가 0인 애들, 1 또는 2형식
			else if((tmp_token.nixbpe & 32) == 0 && (tmp_token.nixbpe & 16) == 0){
				// 1형식
				if(tmp_inst.format==1){
					memset(tmp_hex, 0, sizeof(tmp_hex));
					sprintf(tmp_hex, "%02X", tmp_inst.op);
					location_counter += 1;
				}
				// 2형식
				else if(tmp_inst.format==2){
					int value = 0;
					value |= tmp_inst.op;
					value <<= 4;
					for(int k=0;k<2;k++){
						if(tmp_token.operand[k]==NULL){
							continue;
						}
						else if(*tmp_token.operand[k]=='A')value |= 0;
						else if(*tmp_token.operand[k]=='X')value |= 1;
						else if(*tmp_token.operand[k]=='L')value |= 2;
						else if(*tmp_token.operand[k]=='P')value |= 8;
						else if(*(tmp_token.operand[k]+1)=='W')value |= 9;
						else if(*tmp_token.operand[k]=='B')value |= 3;
						else if(*tmp_token.operand[k]=='S')value |= 4;
						else if(*tmp_token.operand[k]=='T')value |= 5;
						else if(*tmp_token.operand[k]=='F')value |= 6;
						if(k==0)value<<=4;
					}
					memset(tmp_hex, 0, sizeof(tmp_hex));
					sprintf(tmp_hex, "%04X", value);
					location_counter += 2;
				}
			}
			// 3, 4형식
			else if(tmp_inst.format==34){
				// DEFREF에 정의된 변수를 사용했을 때
				for(int k=0;k<ref_cnt;k++){
					if(tmp_token.operand[0]==NULL)continue;
					if(!strcmp(tmp_token.operand[0], tmp_ref[k])){
						strncpy(now_red->name, tmp_ref[k], strlen(tmp_ref[k]));
						strncpy(now_red->base, tmp_base, strlen(tmp_base));
						
						if(tmp_inst.format==34){
							now_red->pos = 5;
							now_red->addr = location_counter + 1;
						}
						else {
							now_red->pos = 6;
							now_red->addr = location_counter;
						}
						now_red->op = '+';
						
						now_red->next = (modification_record*)calloc(1, sizeof(modification_record));
						if(now_red->next==NULL)return -2;
						now_red = now_red->next;
					}
				}
				
				
				// value 변수에 논리연산을 사용해서 오브젝트 코드로만들고 16진수 문자열로 변경할 예정
				int value = 0;
				// opcode와 or연산하고 왼쪽으로 nixbpe의 6비트만큼 민다.
				value |= tmp_inst.op;
				
				value <<= 4;
				// nixbpe와 or연산을하고 12	비트만큼 민다.
				value |= tmp_token.nixbpe;
				value <<= 12;
				// 4형식인 경우 8비트만큼 더 민다.
				if((tmp_token.nixbpe & 1)){
					value <<= 8;
				}
				
				// immdiate를 고려
				if((tmp_token.nixbpe & 16) && !(tmp_token.nixbpe & 32)){
					if(tmp_token.nixbpe & 1)location_counter += 4;
					else location_counter += 3;
					value |= atoi(tmp_token.operand[0]+1);
					memset(tmp_hex, 0, sizeof(tmp_hex));
					sprintf(tmp_hex, "%06X", value);
				}
				else if((tmp_token.nixbpe & 32) && !(tmp_token.nixbpe & 16)){
					if(tmp_token.nixbpe & 1)location_counter += 4;
					else location_counter += 3;
					for(int k=0;k<symbol_table_length;k++){
						if(!strcmp(symbol_table[k]->name, tmp_token.operand[0] + 1) &&
						   !strcmp(symbol_table[k]->base, tmp_base)){
							value |= ((symbol_table[k]->addr - location_counter) & 0b111111111111);
						}
					}
					for(int k=0;k<literal_table_length;k++){
						if(!strcmp(literal_table[k]->literal, tmp_token.operand[0] + 1) &&
						   !strcmp(literal_table[k]->base, tmp_base)){
							value |= ((literal_table[k]->addr - location_counter) & 0b111111111111);
						}
					}
					memset(tmp_hex, 0, sizeof(tmp_hex));
					sprintf(tmp_hex, "%06X", value);
				}
				// 적절한 심보를 찾으면 심볼의 pc relactive값을 넣어줌
				else if((tmp_token.nixbpe & 2)){
					location_counter += 3;
					for(int k=0;k<symbol_table_length;k++){
						if(!strcmp(symbol_table[k]->name, tmp_token.operand[0]) &&
						   !strcmp(symbol_table[k]->base, tmp_base)){
							value |= ((symbol_table[k]->addr - location_counter) & 0b111111111111);
						}
					}
					for(int k=0;k<literal_table_length;k++){
						if(!strcmp(literal_table[k]->literal, tmp_token.operand[0]) &&
						   !strcmp(literal_table[k]->base, tmp_base)){
							value |= ((literal_table[k]->addr - location_counter) & 0b111111111111);
						}
					}
					memset(tmp_hex, 0, sizeof(tmp_hex));
					sprintf(tmp_hex, "%06X", value);
				}
				// 4형식은 심볼의 실제 주소를 넣어줌
				else if((tmp_token.nixbpe & 1)){
					location_counter += 4;
					for(int k=0;k<symbol_table_length;k++){
						if(!strcmp(symbol_table[k]->name, tmp_token.operand[0]) &&
						   !strcmp(symbol_table[k]->base, tmp_base)){
							value |= symbol_table[k]->addr;
						}
					}
					for(int k=0;k<literal_table_length;k++){
						if(!strcmp(literal_table[k]->literal, tmp_token.operand[0]) &&
						   !strcmp(literal_table[k]->base, tmp_base)){
							value |= literal_table[k]->addr;
						}
					}
					memset(tmp_hex, 0, sizeof(tmp_hex));
					sprintf(tmp_hex, "%08X", value);
				}
			}
			
			
			
			left_str = strlen(now->line);
			right_str = strlen(tmp_hex);
			if(left_str+right_str>=100)return -1;
			
			
			
			if(left_str+right_str >= 0x40){
				char tmp[100];
				memset(tmp, 0, sizeof(tmp));
				int now_pos = strlen(now->line)/2;
				sprintf(tmp, "T%06X%02X", pos, now_pos);
				pos += now_pos;
				strcat(tmp, now->line);
				memset(now->line, 0, sizeof(now->line));
				strncpy(now->line, tmp, strlen(tmp));
				
				now->next = (object_code*)calloc(1, sizeof(object_code));
				if(now->next==NULL){
					return -2;
				}
				now = now->next;
			}
			strcat(now->line, tmp_hex);
			
		}
		
	}
	
	now = obj_code;
	int i = 0;
	while(now!=NULL){
		if(i >= pro_cnt)break;
		
		if(*now->line == 'H'){
			memset(tmp_hex, 0, sizeof(tmp_hex));
			sprintf(tmp_hex, "%06X", pro_size[i++]);
			strcat(now->line, tmp_hex);
		}
		
		now = now->next;
	}

	return 0;
}

/**
 * @brief 심볼 테이블을 파일로 출력한다. `symbol_table_dir`이 NULL인 경우 결과를
 * stdout으로 출력한다.
 *
 * @param symbol_table_dir 심볼 테이블을 저장할 파일 경로, 혹은 NULL
 * @param symbol_table 심볼 테이블 주소
 * @param symbol_table_length 심볼 테이블 길이
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 심볼 테이블을 파일로 출력한다. `symbol_table_dir`이 NULL인 경우 결과를
 * stdout으로 출력한다. 명세서에 주어진 출력 예시와 완전히 동일할 필요는 없다.
 */
int make_symbol_table_output(const char *symbol_table_dir,
							 const symbol *symbol_table[],
							 int symbol_table_length) {
	FILE *fp;
	// 쓰기 권한으로 파일입출력을 시작함
	// fp가 NULL이면 file pointer를 표준출력으로 설정
	if(symbol_table_dir==NULL){
		fp = stdout;
	}
	else {
		fp = fopen(symbol_table_dir, "w");
		if(fp==NULL){
			return -1;
		}
	}
	
	for(int i=0;i<symbol_table_length;i++){
		if(!strcmp(symbol_table[i]->name, symbol_table[i]->base)){
			fprintf(fp, "%s\t%X\n",
					symbol_table[i]->name, symbol_table[i]->addr);
		}
		else {
			fprintf(fp, "%s\t%X\t +1 %s\n",
					symbol_table[i]->name, symbol_table[i]->addr, symbol_table[i]->base);
		}
	}
	
	fclose(fp);

	return 0;
}

/**
 * @brief 리터럴 테이블을 파일로 출력한다. `literal_table_dir`이 NULL인 경우
 * 결과를 stdout으로 출력한다.
 *
 * @param literal_table_dir 리터럴 테이블을 저장할 파일 경로, 혹은 NULL
 * @param literal_table 리터럴 테이블 주소
 * @param literal_table_length 리터럴 테이블 길이
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 리터럴 테이블을 파일로 출력한다. `literal_table_dir`이 NULL인 경우 결과를
 * stdout으로 출력한다. 명세서에 주어진 출력 예시와 완전히 동일할 필요는 없다.
 */
int make_literal_table_output(const char *literal_table_dir,
							  const literal *literal_table[],
							  int literal_table_length) {
	FILE *fp;
	// 쓰기 권한으로 파일입출력을 시작함
	// fp가 NULL이면 file pointer를 표준출력으로 설정
	if(literal_table_dir==NULL){
		fp = stdout;
	}
	else {
		fp = fopen(literal_table_dir, "w");
		if(fp==NULL){
			return -1;
		}
	}
	
	for(int i=0;i<literal_table_length;i++){
		fprintf(fp, "%s\t\t%X\n", literal_table[i]->literal, literal_table[i]->addr);
	}
	
	fclose(fp);

	return 0;
}

/**
 * @brief 오브젝트 코드를 파일로 출력한다. `objectcode_dir`이 NULL인 경우 결과를
 * stdout으로 출력한다.
 *
 * @param objectcode_dir 오브젝트 코드를 저장할 파일 경로, 혹은 NULL
 * @param obj_code 오브젝트 코드에 대한 정보를 담고 있는 구조체 주소
 * @return 오류 코드 (정상 종료 = 0)
 *
 * @details
 * 오브젝트 코드를 파일로 출력한다. `objectcode_dir`이 NULL인 경우 결과를
 * stdout으로 출력한다. 명세서의 주어진 출력 결과와 완전히 동일해야 한다.
 * 예외적으로 각 라인 뒤쪽의 공백 문자 혹은 개행 문자의 차이는 허용한다.
 */
int make_objectcode_output(const char *objectcode_dir,
						   const object_code *obj_code) {
	FILE *fp;
	// 쓰기 권한으로 파일입출력을 시작함
	// fp가 NULL이면 file pointer를 표준출력으로 설정
	if(objectcode_dir==NULL){
		fp = stdout;
	}
	else {
		fp = fopen(objectcode_dir, "w");
		if(fp==NULL){
			return -1;
		}
	}
	
	// 오브젝트 코드를 출력
	object_code *now = obj_code;
	while(now!=NULL){
		fputs(now->line, fp);
		fprintf(fp, "\n");
		now = now->next;
	}
	
	fclose(fp);

	return 0;
}
