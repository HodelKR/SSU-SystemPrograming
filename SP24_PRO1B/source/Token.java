import java.util.ArrayList;
import java.util.Arrays;
import java.util.Optional;
import java.util.stream.Collectors;

public class Token {
	/**
	 * 소스 코드 한 줄에 해당하는 토큰을 초기화한다.
	 * 
	 * @param input 소스 코드 한 줄에 해당하는 문자열
	 * @throws RuntimeException 소스 코드 컴파일 오류
	 */
	public Token(String input) throws RuntimeException {
		// TODO: Token 클래스의 field 초기화.
		// Input을 '\t' 단위로 나눔
		String[] tmp = input.split("\t");
		// 각각의 배열요소에 적절하게 접근해서 멤버 변수들을 채움
		for(int i=0;i<tmp.length;i++){
			if(i==0){
				_label = Optional.ofNullable(tmp[i]);
			}
			else if(i==1){
				_operator = Optional.ofNullable(tmp[i]);
			}
			else if(i==2){
				_operands = new ArrayList<>(Arrays.asList(tmp[2].split(",")));
			}
			else if(i==3){
				_comment = Optional.ofNullable(tmp[3]);
			}
		}
		_nixbpe = Optional.of(0);
	}

	// TODO: 필요한 getter 구현하기.

	public String getLabel(){
		return _label.orElse(null);
	}
	public String getOperator(){
		return _operator.orElse(null);
	}
	public ArrayList<String> getOperands(){
		return _operands;
	}
	public Integer getNixbpe(){
		return _nixbpe.orElse(0);
	}
	/**
	 * 토큰의 iNdirect bit가 1인지 여부를 반환한다.
	 * 
	 * @return N bit가 1인지 여부
	 */
	public boolean isN() {
		// TODO: 구현하기.
		int value = 0;
		if(_nixbpe.isPresent()){
			value = _nixbpe.get();
		}
		return (value & 32) == 32;
	}

	/**
	 * 토큰의 Immediate bit가 1인지 여부를 반환한다.
	 * 
	 * @return I bit가 1인지 여부
	 */
	public boolean isI() {
		// TODO: 구현하기.
		int value = 0;
		if(_nixbpe.isPresent()){
			value = _nixbpe.get();
		}
		return (value & 16) == 16;
	}

	/**
	 * 토큰의 indeX bit가 1인지 여부를 반환한다.
	 * 
	 * @return X bit가 1인지 여부
	 */
	public boolean isX() {
		// TODO: 구현하기.
		int value = 0;
		if(_nixbpe.isPresent()){
			value = _nixbpe.get();
		}
		return (value & 8) == 8;
	}

	/*
	 * Base relative는 구현하지 않음.
	 * public boolean isB() {
	 * return false;
	 * }
	 */

	/**
	 * 토큰의 Pc relative bit가 1인지 여부를 반환한다.
	 * 
	 * @return P bit가 1인지 여부
	 */
	public boolean isP() {
		// TODO: 구현하기.
		int value = 0;
		if(_nixbpe.isPresent()){
			value = _nixbpe.get();
        }
        return (value & 2) == 2;
    }

	/**
	 * 토큰의 Extra bit가 1인지 여부를 반환한다.
	 * 
	 * @return E bit가 1인지 여부
	 */
	public boolean isE() {
		// TODO: 구현하기.
		int value = 0;
		if(_nixbpe.isPresent()){
			value = _nixbpe.get();
		}
		return (value & 1) == 1;
	}

	public void updateNixbpe(int v){
		int ret = 0;
		if(_nixbpe.isPresent()){
			ret = _nixbpe.get();
		}
		ret |= v;
		_nixbpe = Optional.of(ret);
	}
	/**
	 * 토큰을 String으로 변환한다. 원활한 디버깅을 위해 기본적으로 제공한 함수이며, Assembler.java에서는 해당 함수를 사용하지
	 * 않으므로 자유롭게 변경하여 사용한다.
	 * 아래 함수는 피연산자에 X가 지정되었더라도 _operands는 X를 저장하지 않고 X bit만 1로 변경한 상태를 가정하였다.
	 */
	@Override
	public String toString() {
		String label = _label.orElse("(no label)");
		String operator = (isE() ? "+ " : "") + _operator.orElse("(no operator)");
		String operand = (isN() ? "@" : "") + (isI() ? "#" : "")
				+ (_operands.isEmpty() ? "(no operand)" : _operands.stream().collect(Collectors.joining("/")))
				+ (isX() ? (_operands.isEmpty() ? "X" : "/X") : "");
		String comment = _comment.orElse("(no comment)");
		return label + '\t' + operator + '\t' + operand + '\t' + comment;
	}

	/** label */
	private Optional<String> _label;

	/** operator */
	private Optional<String> _operator;

	/** operand */
	private ArrayList<String> _operands;

	/** comment */
	private Optional<String> _comment;

	/** nixbpe 비트를 저장하는 변수 */
	private Optional<Integer> _nixbpe;
}
