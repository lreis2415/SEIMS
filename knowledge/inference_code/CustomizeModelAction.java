package action;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.opensymphony.xwork2.ActionSupport;

import inferEngine.ModelCustomize;

public class CustomizeModelAction extends ActionSupport{

	private int tag;
    private String application_purpose;
    private String climate_input;
    private String hydrology_input;
    private String underlying_surface_input;
    private String time_step;
    private String climate_type;
    private String time_scale;
    private String spatial_scale;
    private String spatial_discretization;
    private String hydrologic_regimes;
    private String underlying_surface_type;
    private String particular_process;
    private String timestamp;
    
    
    public String getTimestamp() {
		return timestamp;
	}

	public void setTimestamp(String timestamp) {
		this.timestamp = timestamp;
	}

	private List<String> sorted_process;
    private List<String> sorted_algorithm;
    private List<String> sorted_component;
    
    private HashMap<String,String> condition_value = new HashMap<String,String>();
	public String execute(){ 
		
		this.prepareContext();
		
		ModelCustomize model = new ModelCustomize();
		model.customize_model(this.condition_value,this.timestamp);
		this.sorted_process = model.sorted_processNames;              
		this.sorted_algorithm = model.sorted_algorithmNames;
		this.sorted_component = model.sorted_componentIDs;
		this.prepare_display();
		            
		this.tag = 1;              
		return SUCCESS; 
	}
	            
	private List<Proce_Algori_Compon> pro_alg_coms = new ArrayList<Proce_Algori_Compon>();
	private int total = 1;
	public int getTotal() {
		return total;
	}

	public void setTotal(int total) {
		this.total = total;
	}

	private void prepare_display(){
		
		int len = this.sorted_algorithm.size();
		for(int i =0 ;i<len; i++){
			
			Proce_Algori_Compon pro_alg_com = new Proce_Algori_Compon();
			pro_alg_com.setProcess_name(this.sorted_process.get(i));
			pro_alg_com.setAlgorithm_name(this.sorted_algorithm.get(i));
			pro_alg_com.setComponent_id(this.sorted_component.get(i));
			        
			pro_alg_coms.add(pro_alg_com);
		}
		this.total = len;
		
	}
	
	public List<Proce_Algori_Compon> getPro_alg_coms() {
		return pro_alg_coms;
	}

	public void setPro_alg_coms(List<Proce_Algori_Compon> pro_alg_coms) {
		this.pro_alg_coms = pro_alg_coms;
	}

	private void prepareContext(){
		
		condition_value.put("application purpose", this.application_purpose);
		condition_value.put("climate input", this.climate_input);
		condition_value.put("hydrology input", this.hydrology_input);
		condition_value.put("underlying surface input", this.underlying_surface_input);
		condition_value.put("time step", this.time_step);
		condition_value.put("climate type", this.climate_type);
		condition_value.put("time scale", this.time_scale);
		
		condition_value.put("spatial scale", this.spatial_scale);
		condition_value.put("spatial discretization", this.spatial_discretization);
		condition_value.put("hydrologic regimes", this.hydrologic_regimes);
		condition_value.put("underlying surface type", this.underlying_surface_type);
		condition_value.put("particular process", this.particular_process);

		
	}
	
	public int getTag() {
		return tag;
	}

	public void setTag(int tag) {
		this.tag = tag;
	}

	public String getApplication_purpose() {
		return application_purpose;
	}

	public void setApplication_purpose(String application_purpose) {
		this.application_purpose = application_purpose;
	}

	public String getClimate_input() {
		return climate_input;
	}

	public void setClimate_input(String climate_input) {
		this.climate_input = climate_input;
	}

	public String getHydrology_input() {
		return hydrology_input;
	}

	public void setHydrology_input(String hydrology_input) {
		this.hydrology_input = hydrology_input;
	}

	public String getUnderlying_surface_input() {
		return underlying_surface_input;
	}

	public void setUnderlying_surface_input(String underlying_surface_input) {
		this.underlying_surface_input = underlying_surface_input;
	}

	public String getTime_step() {
		return time_step;
	}

	public void setTime_step(String time_step) {
		this.time_step = time_step;
	}

	public String getClimate_type() {
		return climate_type;
	}

	public void setClimate_type(String climate_type) {
		this.climate_type = climate_type;
	}

	public String getTime_scale() {
		return time_scale;
	}

	public void setTime_scale(String time_scale) {
		this.time_scale = time_scale;
	}

	public String getSpatial_scale() {
		return spatial_scale;
	}

	public void setSpatial_scale(String spatial_scale) {
		this.spatial_scale = spatial_scale;
	}

	public String getSpatial_discretization() {
		return spatial_discretization;
	}

	public void setSpatial_discretization(String spatial_discretization) {
		this.spatial_discretization = spatial_discretization;
	}

	public String getHydrologic_regimes() {
		return hydrologic_regimes;
	}

	public void setHydrologic_regimes(String hydrologic_regimes) {
		this.hydrologic_regimes = hydrologic_regimes;
	}

	public String getUnderlying_surface_type() {
		return underlying_surface_type;
	}

	public void setUnderlying_surface_type(String underlying_surface_type) {
		this.underlying_surface_type = underlying_surface_type;
	}

	public String getParticular_process() {
		return particular_process;
	}

	public void setParticular_process(String particular_process) {
		this.particular_process = particular_process;
	}
	
}
