package modelConfig;


import java.io.FileWriter;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import constant.Constant;

public class ModelConfig {
	
    private static String config_folder_path = Constant.model_config_folder_path;
	

	public static void writeConfig(String filename, String purpose, List<String> timeSeryInputs, 
			List<String> processNames, List<String> algorithmNames, List<String> componentIDs)
	{
		
		try{  
			
			FileWriter fw = new FileWriter(config_folder_path + filename + "config.fig");        
	        fw.write("1 | " + purpose + "\r\n");
	        int index = 1;                                        
	        for(String timeseriesInput:timeSeryInputs){
	        	index++;
	        	fw.write(index + " | TimeSeries_" + timeseriesInput + " | | TSD_RD" + "\r\n");
	        }
	        int len = processNames.size();
	        int pet_index = -1;
	        for(int i =0;i<len; i++){
	        	if(processNames.get(i).equals("PET")){
	        		pet_index = i;
	        		break;
	        	}
	        }
	        if(pet_index >=0){
	        	
	        	index++;
           	 	fw.write(index + " | " + "PET" + " | " + algorithmNames.get(pet_index)  + " | " + componentIDs.get(pet_index) + "\r\n");
	        }
	        

	        for(String timeseriesInput:timeSeryInputs){
	        	
	        	index++;
	        	fw.write(index + " | Interpolation_" + timeseriesInput + "_0" + " | Average uniform | ITP" + "\r\n");
	        }
	        
	        for(int i=0; i<len; i++){
	        	
	        	if(processNames.get(i).equals("PET")){
	        		continue;
	        	}
	        	index++;
	        	fw.write(index + " | " + processNames.get(i) + " | " + algorithmNames.get(i)  + " | " + componentIDs.get(i) + "\r\n");
	        }
	        
	        fw.flush();
	        fw.close();
	        
		}catch(Exception e){
			e.printStackTrace();
		}
 
	}
	
	public static void writeInfile(String modelType, String interval, Date startTime, Date endTime){
		SimpleDateFormat dateformat=new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		try{
			FileWriter fw = new FileWriter(config_folder_path + "file.in");        
	        fw.write("MODE|" + modelType + "\r\n");
	        fw.write("INTERVAL|" + interval + "\r\n");
	        fw.write("STARTTIME|" + dateformat.format(startTime) + "\r\n");
	        fw.write("ENDTIME|" + dateformat.format(endTime) + "\r\n");
	        fw.flush();
	        fw.close();
		}catch(Exception e){
			e.printStackTrace();
		}
		
	}
	
	public static void writeOutfile(Date startTime, Date endTime){
		SimpleDateFormat dateformat=new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		try{
			FileWriter fw = new FileWriter(config_folder_path +  "file.out");        
	        fw.write("OUTPUTID | QRECH" + "\r\n");
	        fw.write("SUBBASINCOUNT | 1" + "\r\n");
	        fw.write("STARTTIME|" + dateformat.format(startTime) + "\r\n");
	        fw.write("ENDTIME|" + dateformat.format(endTime) + "\r\n");
	        fw.write("FILENAME | Q.txt" + "\r\n");
	        fw.flush();
	        fw.close();
		}catch(Exception e){
			e.printStackTrace();
		}
		
	}
	
	private static void test_writeConfig(){
		
		String purpose = "runoff simulation"; 
		List<String> timeSeryInputs = new ArrayList<String>();
		List<String> processNames = new ArrayList<String>();
		List<String> algorithmNames = new ArrayList<String>();
		List<String> componentIDs  = new ArrayList<String>();
		
		timeSeryInputs.add("TMAX");
		timeSeryInputs.add("TMIN");
		timeSeryInputs.add("P");
		
		processNames.add("PET");
		processNames.add("Interception");
		processNames.add("Snow Melt");
		
		algorithmNames.add("Hargreaves");
		algorithmNames.add("interception algorithm in WetSpa");
		algorithmNames.add("Degree-Day");
		
		componentIDs.add("PET_H");
		componentIDs.add("PI_MSM");
		componentIDs.add("SNO_DD");
		               
		writeConfig("test", purpose, timeSeryInputs,    
				processNames, algorithmNames, componentIDs);
	}                   
	
    public static void main(String[] args){
    	
    	test_writeConfig();
    
    }   
}
