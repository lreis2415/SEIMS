package constant;

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URISyntaxException;
import java.util.Properties;

/**
 * 
 * @author jjc
 *
 */
public class Constant { 
	 
	public static final String model_ruleml_path = GetPropertyValue("config.properties", "model_ruleml_path");
	public static final String process_ruleml_path =GetPropertyValue("config.properties", "process_ruleml_path"); 
	public static final String algorithm_ruleml_path = GetPropertyValue("config.properties", "algorithm_ruleml_path");
	public static final String component_rdf_folder_path = GetPropertyValue("config.properties", "component_rdf_folder_path");
	public static final String algorithm_rdf_path = GetPropertyValue("config.properties", "algorithm_rdf_path");
	public static final String model_config_folder_path = GetPropertyValue("config.properties", "model_config_folder_path");
	public static final String xml_path = GetPropertyValue("config.properties", "xml_path");
	public static final String original_rdf_path = GetPropertyValue("config.properties", "original_rdf_path");
	public static final String original_variable_path = GetPropertyValue("config.properties", "original_variable_path");
	
	
	public static String GetPropertyValue(String file,String key){
		Properties props = new Properties();
		InputStream ips = null;  
		try {
			ips = new BufferedInputStream(new FileInputStream(
					Constant.class.getResource("/").toURI().getPath() + file));
			props.load(ips);
			String value = props.getProperty(key);
			return value.trim();     
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return null;
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		} catch (URISyntaxException e) {
			e.printStackTrace();
			return null;
		}finally{
			try{
				if(ips!=null){				
					ips.close();				
					ips = null;			
				}
				}catch(Exception e){
					e.printStackTrace();
				}
			}
	}
}

