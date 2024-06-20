import java.io.*;
import java.util.HashMap;
import java.util.Optional;

public class InstructionTable {
	/**
	 * 기계어 목록 파일을 읽어, 기계어 목록 테이블을 초기화한다.
	 * 
	 * @param instFileName 기계어 목록이 적힌 파일
	 * @throws FileNotFoundException 기계어 목록 파일 미존재
	 * @throws IOException           파일 읽기 실패
	 */
	public InstructionTable(String instFileName) throws FileNotFoundException, IOException {
		instructionMap = new HashMap<String, InstructionInfo>();

		// 파일과 버퍼리더 정의
		File file;
		BufferedReader bufReader;

		try{
			file = new File(instFileName);
			bufReader = new BufferedReader(new FileReader(file));
		}catch (FileNotFoundException e){
			throw new FileNotFoundException();
		}

        String line = "";
		while ((line = bufReader.readLine()) != null){
			InstructionInfo instructionInfo = new InstructionInfo(line);
			instructionMap.put(instructionInfo.getName(), instructionInfo);
		}
		bufReader.close();
		// TODO: fileName의 파일을 열고, 해당 내용을 파싱하여 instructionMap에 저장하기.
	}

	/**
	 * 기계어 목록 테이블에서 특정 기계어를 검색한다.
	 *
	 * @param instructionName 검색할 기계어 명칭
	 * @return 기계어 정보. 없을 경우 empty
	 */
	public Optional<InstructionInfo> search(String instructionName) {
		if(instructionName.charAt(0)=='+'){
			instructionName = instructionName.substring(1);
		}
		for (String key: instructionMap.keySet()) {
			if(key.equals(instructionName)){
				return Optional.ofNullable(instructionMap.get(key));
			}
		}
		// TODO: instructionMap에서 instructionName에 해당하는 명령어의 정보 반환하기.
		return Optional.empty();
	}

	/** 기계어 목록 테이블. key: 기계어 명칭, value: 기계어 정보 */
	private HashMap<String, InstructionInfo> instructionMap;
}