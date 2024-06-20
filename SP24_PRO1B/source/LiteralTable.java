import java.util.HashMap;
import java.util.Optional;

public class LiteralTable {
	/**
	 * 리터럴 테이블을 초기화한다.
	 */
	public LiteralTable() {
		literalMap = new HashMap<String, Literal>();
	}

	/**
	 * 리터럴을 리터럴 테이블에 추가한다.
	 * 
	 * @param literal 추가할 리터럴
	 * @throws RuntimeException 비정상적인 리터럴 서식
	 */
	public void putLiteral(String name, String base) throws RuntimeException {
		// TODO: 리터럴을 literalMap에 추가하기.
		Literal literal = new Literal(name, base);
		boolean flag = true;
		for(var x : literalMap.keySet()){
			if(x.equals(name)){
				flag = false;
				break;
			}
		}
		if(flag){
			literalMap.put(name, literal);
		}
	}
	// TODO: 추가로 필요한 method 구현하기.

	// 리터럴을 찾아서 주소를 세팅
	public int update(int address){
		int ret = 0;
		for(var x : literalMap.keySet()){
			Literal tmp = literalMap.get(x);
			if(tmp.getAddress().orElse(-1)==-1){
				tmp.setAddress(address);
				ret += tmp.getSize();
				address += tmp.getSize();
			}
		}
		return ret;
	}

	/**
	 * 리터럴 테이블을 String으로 변환한다.
	 */
	@Override
	public String toString() {
		// TODO: 구현하기. Literal 객체의 toString을 활용하자.
		StringBuilder ret = new StringBuilder();
		for(String x : literalMap.keySet()){
			ret.append(literalMap.get(x).toString());
			ret.append("\n");
		}
		return ret.toString();
	}

	public HashMap<String, Literal> getLiteralMap(){
		return literalMap;
	}
	/** 리터럴 맵. key: 리터럴 String, value: 리터럴 객체 */
	private HashMap<String, Literal> literalMap;
}

class Literal {
	/**
	 * 리터럴 객체를 초기화한다.
	 * 
	 * @param literal 리터럴 String
	 */
	public Literal(String literal, String base) {
		// TODO: 리터럴 객체 초기화.
		_literal = literal;
		_base = base;
		_address = Optional.of(-1);
		if(literal.charAt(1)=='X'){
			_size = (literal.length() - 4) / 2;
		}
		else if(literal.charAt(1)=='C'){
			_size = literal.length() - 4;
		}
	}

	// 주소를 세팅
	public void setAddress(int address){
		_address = Optional.of(address);
	}
	/**
	 * 리터럴 String을 반환한다.
	 * 
	 * @return 리터럴 String
	 */
	public String getLiteral() {
		return _literal;
	}

	/**
	 * 리터럴의 주소를 반환한다. 주소가 지정되지 않은 경우, Optional.empty()를 반환한다.
	 * 
	 * @return 리터럴의 주소
	 */
	public Optional<Integer> getAddress() {
		return _address;
	}
	public int getSize(){
		return _size;
	}

	public String getBase() { return _base; }

	// TODO: 추가로 선언한 field에 대한 getter 작성하기.

	/**
	 * 리터럴을 String으로 변환한다. 리터럴의 address에 관한 정보도 리턴값에 포함되어야 한다.
	 */
	@Override
	public String toString() {
		// TODO: 리터럴을 String으로 표현하기.
		return getLiteral() + "\t" + Integer.toHexString(getAddress().orElse(0)).toUpperCase();
	}

	/** 리터럴 String */
	private String _literal;

	private int _size;

	private String _base;
	/** 리터럴 주소. 주소가 지정되지 않은 경우 empty */
	private Optional<Integer> _address;

	// TODO: 추가로 필요한 field 선언하기.
}