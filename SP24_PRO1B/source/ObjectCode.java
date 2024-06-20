import java.util.ArrayList;

public class ObjectCode {
	public ObjectCode() {
		// TODO: 초기화.
		_code = new ArrayList<String>();
	}

	/**
	 * ObjectCode 객체를 String으로 변환한다. Assembler.java에서 오브젝트 코드를 출력하는 데에 사용된다.
	 */
	@Override
	public String toString() {
		// TODO: toString 구현하기.
		StringBuilder ret = new StringBuilder();
		for(String str : _code){
			ret.append(str);
			ret.append("\n");
		}
		return ret.toString();
	}

	// TODO: private field 선언.
	public void addString(String line){
		_code.add(line);
	}
	private ArrayList<String> _code;
}
