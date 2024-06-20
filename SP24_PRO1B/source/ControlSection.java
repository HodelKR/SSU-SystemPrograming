import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Optional;

public class ControlSection {
	/**
	 * pass1 작업을 수행한다. 기계어 목록 테이블을 통해 소스 코드를 토큰화하고, 심볼 테이블 및 리터럴 테이블을 초기화환다.
	 * 
	 * @param instTable 기계어 목록 테이블
	 * @param input     하나의 control section에 속하는 소스 코드. 마지막 줄은 END directive를 강제로
	 *                  추가하였음.
	 * @throws RuntimeException 소스 코드 컴파일 오류
	 */
	public ControlSection(InstructionTable instTable, ArrayList<String> input) throws RuntimeException {
		// TODO: pass1 수행하기.
		// Instruction Table을 가져오고 각각의 생성자 호출
		_instTable = instTable;
		_tokens = new ArrayList<Token>();
		_symbolTable = new SymbolTable();
		_literalTable = new LiteralTable();

		// Input을 Token으로 저장함
		for(String line : input){
			Token token = new Token(line);
			_tokens.add(token);
		}

		int locctr = 0;
		String base = null;
		for(Token now : _tokens){
			// 주석인 경우
			if(now.getLabel().equals(".")){
				continue;
			}

			// 각 프로그램의 첫 시작인 부분 Location Counter를 정의하는 부분
			// START, CSECT
			try {
				if (now.getOperator().equals("START")) {
					locctr = Integer.parseInt(now.getOperands().getFirst());
					base = now.getLabel();
				}
				if (now.getOperator().equals("CSECT")) {
					locctr = 0;
					base = now.getLabel();
				}
			}catch (Exception e){
				System.out.println("[PASS 1] START, CSECT Error");
				throw new RuntimeException(e);
			}

			// Literal Table을 업데이트
			try {
				if (now.getOperator().equals("LTORG")) {
					locctr += _literalTable.update(locctr);
				}
			}catch (Exception e){
				System.out.println("[PASS 1] LTORG Error");
				throw new RuntimeException(e);
			}

			// Literal Table을 채운다.
			try {
				if(now.getOperands()!=null && now.getOperands().getFirst()!=null) {
					if (!now.getOperands().getFirst().isEmpty() && now.getOperands().getFirst().charAt(0) == '=') {
						_literalTable.putLiteral(now.getOperands().getFirst(), base);
					}
				}
			}catch (Exception e){
				System.out.println("[PASS 1] Literal Table fill Error");
				throw new RuntimeException(e);
			}

			// Symbol Table을 채우는 부분
			try {
				if (!now.getLabel().isEmpty()) {
					if (now.getOperator().equals("EQU"))
						_symbolTable.putLabel(now.getLabel(), locctr, now.getOperands().getFirst(), base);
					else
						_symbolTable.putLabel(now.getLabel(), locctr, base);
				}
			}catch (Exception e){
				System.out.println("[PASS 1] Symbol Table fill Error");
				throw new RuntimeException(e);
			}

			try {
				if(now.getOperator().equals("EXTREF")){
					for(String str : now.getOperands()){
						_symbolTable.putLabel(str, -1, base);
					}
				}
			}catch (Exception e){
				System.out.println("[PASS 1] EXTREF Error");
				throw new RuntimeException(e);
			}

			// Location Counter를 증가하는 부분 1
			// BYTE, WORD, RESW, RESB
			try {
				if (now.getOperator().equals("BYTE")) {
					if (now.getOperands().getFirst().charAt(0) == 'X') {
						locctr += (now.getOperands().getFirst().length() - 3) / 2;
					} else if (now.getOperands().getFirst().charAt(0) == 'C') {
						locctr += now.getOperands().getFirst().length() - 3;
					}
				} else if (now.getOperator().equals("WORD")) {
					locctr += 3;
				} else if (now.getOperator().equals("RESW")) {
					locctr += Integer.parseInt(now.getOperands().getFirst()) * 3;
				} else if (now.getOperator().equals("RESB")) {
					locctr += Integer.parseInt(now.getOperands().getFirst());
				}
			}catch (Exception e){
				System.out.println("[PASS 1] Location Counter 1 Error");
				throw new RuntimeException(e);
			}

			try {
				// Location Counter를 증가하는 부분 2
				// 명령어 1, 2, 3, 4 형식
				var inst = _instTable.search(now.getOperator());
				if (inst.isPresent()) {
					InstructionInfo instructionInfo = inst.get();
					if (instructionInfo.getFormat() == 1) {
						locctr += 1;
					} else if (instructionInfo.getFormat() == 2) {
						locctr += 2;
					} else if (instructionInfo.getFormat() == 34) {
						if (now.getOperator().charAt(0) == '+') {
							locctr += 4;
						} else {
							locctr += 3;
						}

						if(!now.getOperands().isEmpty()){
							if(!now.getOperands().getFirst().isEmpty() && now.getOperands().getFirst().charAt(0)=='#') {
								now.updateNixbpe(16);
							}
							else if(!now.getOperands().getFirst().isEmpty() && now.getOperands().getFirst().charAt(0)=='@') {
								now.updateNixbpe(34);
							}
							else if(now.getOperator().charAt(0)=='+'){
								now.updateNixbpe(49);
							}
							else if(now.getOperands().getFirst().length()>1){
								now.updateNixbpe(50);
							}
							else{
								now.updateNixbpe(48);
							}
						}
						for(var x : now.getOperands()){
							if(x.equals("X")){
								now.updateNixbpe(8);
							}
						}
					}
				}
			}catch (Exception e){
				System.out.println("[PASS 1] Location Counter 2 Error");
				throw new RuntimeException(e);
			}
		}
		// 프로그램의 끝을 만났을 때 실행
		try {
			_literalTable.update(locctr);
		}catch (Exception e){
			System.out.println("[PASS 1] Last literal update Error");
			throw new RuntimeException(e);
		}
	}

	/**
	 * pass2 작업을 수행한다. pass1에서 초기화한 토큰 테이블, 심볼 테이블 및 리터럴 테이블을 통해 오브젝트 코드를 생성한다.
	 * 
	 * @return 해당 control section에 해당하는 오브젝트 코드 객체
	 * @throws RuntimeException 소스 코드 컴파일 오류
	 */
	public ObjectCode buildObjectCode() throws RuntimeException {
		ObjectCode objCode = new ObjectCode();
		// TODO: pass2 수행하기.

		ArrayList<String> strList = new ArrayList<String>();
		ArrayList<String> mr = new ArrayList<String>();
		int proidx = 0;
		StringBuilder tmp;

		int startFlag = 0;
		int locctr = 0;
		int textctr = 0;
		boolean ltorgFlag = true;
		String base = null;
		StringBuilder text = new StringBuilder();

		for(Token now : _tokens) {
			if (now.getLabel().equals(".")) {
				continue;
			}
			// START 시작부분의 역할에 맞게 처리해줌
			try {
				if (now.getOperator().equals("START")) {
					proidx = strList.size();
					base = now.getLabel();
					startFlag = 1;
					locctr = Integer.parseInt(now.getOperands().getFirst(), 16);
					tmp = new StringBuilder("H");
					tmp.append(now.getLabel());
					tmp.append("\t");
					tmp.append("0".repeat(Math.max(0, 6 - now.getOperands().getFirst().length())));
					tmp.append(now.getOperands().getFirst());
					strList.add(tmp.toString());
				}
			} catch (Exception e) {
				System.out.println("[PASS 2] START Error");
				throw new RuntimeException(e);
			}

			// EXTDEF 정의한 레퍼런스를 심볼테이블에서 주소를 가져와서 오브젝트 코드에 적음
			try {
				if (now.getOperator().equals("EXTDEF")) {
					tmp = new StringBuilder("D");
					for(String str : now.getOperands()){
						tmp.append(str);
						String address = Integer.toHexString(_symbolTable.getAddress(str, base)).toUpperCase();
						tmp.append("0".repeat(Math.max(0, 6 - address.length())));
						tmp.append(address);
					}
					strList.add(tmp.toString());
				}
			} catch (Exception e) {
				System.out.println("[PASS 2] EXTDEF Error");
				throw new RuntimeException(e);
			}

			// EXTREF 참조한 레퍼런스들을 오브젝트 코드에 적음
			try {
				if (now.getOperator().equals("EXTREF")) {
					tmp = new StringBuilder("R");
					for(String str : now.getOperands()){
						String s = String.format("%-" + 6 + "s", str);
						tmp.append(s);
					}
					strList.add(tmp.toString());
				}
			} catch (Exception e) {
				System.out.println("[PASS 2] EXTREF Error");
				throw new RuntimeException(e);
			}

			// Text 1 Location Counter를 증가시키는 로직을 명령어 1,2,3,4형식과 분리해서 try 함 디버깅을 편하게 하기 위함
			try {
				if (now.getOperator().equals("BYTE")) {
					StringBuilder byteStr = new StringBuilder();
					if (now.getOperands().getFirst().charAt(0) == 'X') {
						locctr += (now.getOperands().getFirst().length() - 3) / 2;
						String s = now.getOperands().getFirst().substring(2,now.getOperands().getFirst().length()-1);
						byteStr.append(s);
					} else if (now.getOperands().getFirst().charAt(0) == 'C') {
						locctr += now.getOperands().getFirst().length() - 3;
						String s = now.getOperands().getFirst().substring(2,now.getOperands().getFirst().length()-1);
						for(char c : s.toCharArray()){
							byteStr.append(Integer.toHexString((int) c));
						}
					}
					int a = strList.size();
					text = getStringBuilder(strList, textctr, text, byteStr, false);
					if(strList.size()>a){
						textctr = locctr;
						textctr -= byteStr.length()/2;
					}
				} else if (now.getOperator().equals("WORD")) {
					text.append("000000");
					String[] parts = now.getOperands().getFirst().split("[-+*/]");
					StringBuilder m;
					m = new StringBuilder("M");
					String s = Integer.toHexString(locctr).toUpperCase();
					m.append("0".repeat(Math.max(0, 6 - s.length())));
					m.append(s);m.append("06");m.append("+");m.append(parts[0]);
					mr.add(m.toString());
					m = new StringBuilder("M");
					m.append("0".repeat(Math.max(0, 6 - s.length())));
					m.append(s);m.append("06");m.append("-");m.append(parts[1]);
					mr.add(m.toString());
					locctr += 3;
				} else if (now.getOperator().equals("RESW")) {
					locctr += Integer.parseInt(now.getOperands().getFirst()) * 3;
				} else if (now.getOperator().equals("RESB")) {
					locctr += Integer.parseInt(now.getOperands().getFirst());
				}
			}catch (Exception e){
				System.out.println("[PASS 2] Text 1 Error");
				throw  new RuntimeException(e);
			}
			// Text 2 명령어 1,2,3,4 형식을 적절하게 처리해줌
			try {
				var inst = _instTable.search(now.getOperator());
				StringBuilder textStr = new StringBuilder();
				if (inst.isPresent()) {
					InstructionInfo instructionInfo = inst.get();
					if (instructionInfo.getFormat() == 1) {
						StringBuilder s = new StringBuilder(Integer.toHexString(instructionInfo.getOpcode()).toUpperCase());
						for(int i=0;i<2-s.length();i++) s.insert(0, "0");
						textStr.append(s);
						locctr += 1;
					} else if (instructionInfo.getFormat() == 2) {
						StringBuilder s = new StringBuilder(Integer.toHexString(instructionInfo.getOpcode()).toUpperCase());
						for(int i=0;i<2-s.length();i++) s.insert(0, "0");
						textStr.append(s);
						// Register 각각에 매칭
						HashMap<String, Integer> register = new HashMap<String, Integer>() {{
							put("A", 0);put("X", 1);put("L", 2);put("PC", 8);put("SW", 9);
							put("B", 3);put("S", 4);put("T", 5);put("F", 6);
						}};
						if(now.getOperands().getFirst()!=null){
							textStr.append(Integer.toHexString(register.get(now.getOperands().getFirst())).toUpperCase());
						}
						else textStr.append("0");
						if(now.getOperands().size()>1){
							textStr.append(Integer.toHexString(register.get(now.getOperands().get(1))).toUpperCase());
						}
						else textStr.append("0");
						locctr += 2;
					} else if (instructionInfo.getFormat() == 34) {
						// 오류 처리
						if(instructionInfo.getNumberOfOperand()>0){
							var x = _symbolTable.searchSymbol(now.getOperands().getFirst(), base);
							String s = now.getOperands().getFirst();
							char c = s.charAt(0);
							if(c!='#' && c!='@' && c!='=' && x.isEmpty()){
								System.out.println("(no label)\t" + now.toString());
							}
						}
						// 명령어 3, 4형식에 맞게 비트연산하면서 명령어를 Hex로 바꾸는 것을 적절하게 처리함
						long v = 0;
						v = instructionInfo.getOpcode();
						v <<= 4;
						v |= now.getNixbpe();

						String operand = now.getOperands().getFirst();
						var symbol = _symbolTable.searchSymbol(operand, base);
						var l = _literalTable.getLiteralMap().get(operand);
						if (now.getOperator().charAt(0) == '+') {
							locctr += 4;
							v <<= 20;
							if(symbol.isPresent()){
								if(symbol.get().getAddress()==-1){
									StringBuilder m;
									m = new StringBuilder("M");
									String s = Integer.toHexString(locctr).toUpperCase();
									m.append("0".repeat(Math.max(0, 6 - s.length())));
									m.append(s);m.append("05");m.append("+");m.append(symbol.get().getName());
									mr.add(m.toString());
								}
								else {
									v |= symbol.get().getAddress();
								}
							}
							else if(l != null && l.getAddress().isPresent()){
								v |= (l.getAddress().get() - locctr);
							}
							else if(now.getOperands()!=null){
								if(now.getOperands().getFirst().length()>1 && now.getOperands().getFirst().charAt(0)=='#') {
									String s = now.getOperands().getFirst().substring(1);
									v |= Integer.parseInt(s);
								}
							}
						} else {
							locctr += 3;
							v <<= 12;
							if(symbol.isPresent()) {
								if (symbol.get().getAddress() == -1) {
									StringBuilder m;
									m = new StringBuilder("M");
									String s = Integer.toHexString(locctr).toUpperCase();
									m.append("0".repeat(Math.max(0, 6 - s.length())));
									m.append(s);m.append("05");m.append("+");m.append(symbol.get().getName());
									mr.add(m.toString());
								}
								else {
									v |= (symbol.get().getAddress() - locctr) & 0xFFF;
								}
							}
							else if(l != null && l.getAddress().isPresent()){
								v |= (l.getAddress().get() - locctr) & 0xFFF;
							}
							else if(now.getOperands()!=null){
								if(now.getOperands().getFirst().length()>1 && now.getOperands().getFirst().charAt(0)=='#') {
									String s = now.getOperands().getFirst().substring(1);
									v |= Integer.parseInt(s);
								}
							}
						}
						textStr.append(Long.toHexString(v).toUpperCase());
					}
					if(textStr.length()%2==1)textStr = new StringBuilder("0" + textStr);
					int a = strList.size();
					text = getStringBuilder(strList, textctr, text, textStr, false);
					if(strList.size()>a){
						textctr = locctr;
						textctr -= textStr.length()/2;
					}
				}
			}catch (Exception e){
				System.out.println("[PASS 2] Text 2 Error");
				throw new RuntimeException(e);
			}


			// LTORG Literal을 처리해줌
			try {
				if (now.getOperator().equals("LTORG")) {
					StringBuilder literalStr = new StringBuilder();
					for(String str : _literalTable.getLiteralMap().keySet()) {
						Literal literal = _literalTable.getLiteralMap().get(str);
						if (literal.getBase().equals(base)) {
							String l = literal.getLiteral().substring(3, literal.getLiteral().length() - 1);
							if (literal.getLiteral().charAt(1) == 'X') {
								literalStr.append(l);
							} else if (literal.getLiteral().charAt(1) == 'C') {
								for (char c : l.toCharArray()) {
									literalStr.append(Integer.toHexString((int) c));
								}
							}
						}
					}
					int a = strList.size();
					text = getStringBuilder(strList, textctr, text, literalStr, true);
					if(strList.size()>a){
						textctr = locctr;
						textctr -= literalStr.length()/2;
					}
					locctr += literalStr.length() / 2;
					ltorgFlag = false;
				}
			} catch (Exception e) {
				System.out.println("[PASS 2] LTORG Error");
				throw new RuntimeException(e);
			}

			// END Literal과 modification Recode를 처리해줌
			try {
				if (now.getOperator().equals("END")) {
					if(ltorgFlag) {
						StringBuilder literalStr = new StringBuilder();
						for (String str : _literalTable.getLiteralMap().keySet()) {
							Literal literal = _literalTable.getLiteralMap().get(str);
							if (literal.getBase().equals(base)) {
								String l = literal.getLiteral().substring(3, literal.getLiteral().length() - 1);
								if (literal.getLiteral().charAt(1) == 'X') {
									literalStr.append(l);
								} else if (literal.getLiteral().charAt(1) == 'C') {
									for (char c : l.toCharArray()) {
										literalStr.append(Integer.toHexString((int) c));
									}
								}
							}
						}
						int a = strList.size();
						text = getStringBuilder(strList, textctr, text, literalStr, false);
						if(strList.size()>a){
							textctr = locctr;
							textctr -= literalStr.length()/2;
						}
						locctr += literalStr.length() / 2;
					}
					ltorgFlag = true;
					if(!text.isEmpty()){
						StringBuilder textHeader = new StringBuilder("T");
						String hexctr = Integer.toHexString(textctr).toUpperCase();
						textHeader.append("0".repeat(Math.max(0, 6 - hexctr.length())));
						textHeader.append(hexctr);
						String s = Integer.toHexString(text.length()/2).toUpperCase();
						if(s.length()%2==1)textHeader.append("0");
						textHeader.append(s);
						textHeader.append(text);
						strList.add(textHeader.toString());
						textctr = locctr;
						text = new StringBuilder();
					}

					StringBuilder p;
					p = new StringBuilder(strList.get(proidx));
					String loc = Integer.toHexString(locctr).toUpperCase();
					p.append("0".repeat(Math.max(0, 6 - loc.length())));
					p.append(loc);
					strList.set(proidx, p.toString());

					strList.addAll(mr);
					mr.clear();

					tmp = new StringBuilder("E");
					if(startFlag==1){
						startFlag = 0;
						String s = Integer.toHexString(Integer.parseInt(_tokens.getFirst().getOperands().getFirst(), 16));
						tmp.append("0".repeat(Math.max(0, 6 - s.length())));
						tmp.append(s);
					}
					strList.add(tmp.toString());
				}
			} catch (Exception e) {
				System.out.println("[PASS 2] END Error");
				throw new RuntimeException(e);
			}

			// CSECT
			try {
				if (now.getOperator().equals("CSECT")) {
					proidx = strList.size();
					locctr = 0;
					textctr = 0;
					base = now.getLabel();

					tmp = new StringBuilder("H");
					tmp.append(now.getLabel());
					tmp.append("\t000000");
					strList.add(tmp.toString());
				}
			} catch (Exception e) {
				System.out.println("[PASS 2] CSECT Error");
				throw new RuntimeException(e);
			}
		}
		// objCode 출력
		for(String str : strList){
			objCode.addString(str);
		}
		return objCode;
	}

	private StringBuilder getStringBuilder(ArrayList<String> strList, int textctr, StringBuilder text, StringBuilder byteStr, boolean ltorg) {
		if(ltorg){
			if((text.length() + byteStr.length()) >= 0x20){
				StringBuilder textHeader = new StringBuilder("T");
				String hexctr = Integer.toHexString(textctr).toUpperCase();
				textHeader.append("0".repeat(Math.max(0, 6 - hexctr.length())));
				textHeader.append(hexctr);
				String s = Integer.toHexString(text.length()/2).toUpperCase();
				if(s.length()%2==1)textHeader.append("0");
				textHeader.append(s);
				textHeader.append(text);
				strList.add(textHeader.toString());
				text = new StringBuilder();
			}
		}
		if((text.length() + byteStr.length()) >= 0x40){
			StringBuilder textHeader = new StringBuilder("T");
			String hexctr = Integer.toHexString(textctr).toUpperCase();
			textHeader.append("0".repeat(Math.max(0, 6 - hexctr.length())));
			textHeader.append(hexctr);
			String s = Integer.toHexString(text.length()/2).toUpperCase();
			if(s.length()%2==1)textHeader.append("0");
			textHeader.append(s);
			textHeader.append(text);
			strList.add(textHeader.toString());
			text = new StringBuilder();
		}
		text.append(byteStr);
		return text;
	}


	/**
	 * 심볼 테이블을 String으로 변환하여 반환한다. Assembler.java에서 심볼 테이블을 출력하는 데에 사용된다.
	 * 
	 * @return 문자열로 변경된 심볼 테이블
	 */
	public String getSymbolString() {
		return _symbolTable.toString();
	}

	/**
	 * 리터럴 테이블을 String으로 변환하여 반환한다. Assembler.java에서 리터럴 테이블을 출력하는 데에 사용된다.
	 * 
	 * @return 문자열로 변경된 리터럴 테이블
	 */
	public String getLiteralString() {
		return _literalTable.toString();
	}

	/** 기계어 목록 테이블 */
	private InstructionTable _instTable;

	/** 토큰 테이블 */
	private ArrayList<Token> _tokens;

	/** 심볼 테이블 */
	private SymbolTable _symbolTable;

	/** 리터럴 테이블 */
	private LiteralTable _literalTable;
}