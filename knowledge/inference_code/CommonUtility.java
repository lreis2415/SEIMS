package engineUtility;

import java.util.Arrays;
import java.util.List;


public class CommonUtility {
	
	public boolean lessthan(String var, String ind){
		boolean flag = false;
		double num1 = Double.valueOf(var);
		double num2 = Double.valueOf(ind);
		flag = (num1<num2?true:false);
		//System.out.println(flag);
		return  flag;	
	}
	
	public boolean greaterthan(String var, String ind){
		boolean flag = false;
		double num1 = Double.valueOf(var);
		double num2 = Double.valueOf(ind);
		flag = (num1>num2?true:false);
		return  flag;	
	}
	
	public boolean are(String var, String ind){
		
		if(ind.contains(",")){
			boolean flag = true;
			
			List<String> var_list = Arrays.asList(var.split(","));
			List<String> ind_list = Arrays.asList(ind.split(","));
			int len = ind_list.size();
			for(int i=0; i<len; i++){
				
				if(!var_list.contains(ind_list.get(i))){
					flag = false;
					break;
				}
			}
		
			return flag;
		}else{
			boolean flag = false;
			flag = (var.equals(ind)?true:false);
			return flag;		
		}
	}
	
	public boolean is(String var, String ind){
		if(ind.contains("/")){
			boolean flag = false;
			flag = (ind.contains(var)?true:false);
			return flag;
		}else{
			boolean flag = false;
			flag = (var.equals(ind)?true:false);
			return flag;		
		}
		
	}

	public boolean no_contain_input(String var, String ind){
		
		boolean flag = false;
		
		String[] var_tmps = null;
		String[] ind_tmps = null;
        if(var.contains(",")){
        	var_tmps = var.split(",");
        }else{
        	var_tmps = new String[1];
        	var_tmps[0] = var;
        }
        
        if(ind.contains(",")){
        	ind_tmps = ind.split(",");
        }else{
        	ind_tmps = new String[1];
        	ind_tmps[0] = ind;
        }
        
        
        int var_len = var_tmps.length;
        int ind_len = ind_tmps.length;
        
        int count = 0;
        for(int i =0; i<ind_len; i++){
        	
        	for(int j =0; j<var_len; j++){
        		if(ind_tmps[i].equals(var_tmps[j])){
        			count ++;
        			break;
        		}
        	}
        }
        if(count == 0){
        	flag = true;//代表不包含
        }
       
		return flag;
	}
	
	public boolean contain_input(String var, String ind){
		
		boolean flag = false;
		
		String[] var_tmps = null;
		String[] ind_tmps = null;
        if(var.contains(",")){
        	var_tmps = var.split(",");
        }else{
        	var_tmps = new String[1];
        	var_tmps[0] = var;
        }
        
        if(ind.contains(",")){
        	ind_tmps = ind.split(",");
        }else{
        	ind_tmps = new String[1];
        	ind_tmps[0] = ind;
        }
        
        
        int var_len = var_tmps.length;
        int ind_len = ind_tmps.length;
        
        if(var_len < ind_len){
        	
        	flag = false;
        }
        
        for(int i =0; i<ind_len; i++){
        	flag = false;
        	for(int j =0; j<var_len; j++){
        		if(ind_tmps[i].equals(var_tmps[j])){
        			
        			flag = true;
        			break;
        		}
        	}
        	if(flag == false) break;
        	
        }
       
		return flag;
	}
	
	public boolean contain_output(String var, String ind){
		
		boolean flag = false;
		
		String[] var_tmps = null;
		String[] ind_tmps = null;
        if(var.contains(",")){
        	var_tmps = var.split(",");
        }else{
        	var_tmps = new String[1];
        	var_tmps[0] = var;
        }
        
        if(ind.contains(",")){
        	ind_tmps = ind.split(",");
        }else{
        	ind_tmps = new String[1];
        	ind_tmps[0] = ind;
        }
        
        
        int var_len = var_tmps.length;
        int ind_len = ind_tmps.length;
        
        if(var_len > ind_len){
        	
        	flag = false;
        }
        
        for(int i =0; i<var_len; i++){
        	flag = false;
        	for(int j =0; j<ind_len; j++){
        		if(var_tmps[i].equals(ind_tmps[j])){
        			
        			flag = true;
        			break;
        		}
        	}
        	if(flag == false) break;
        	
        }
       
		return flag;
	}
	
	public boolean locate(String var, String ind){
		boolean flag = false;
		flag = (var.equals(ind)?true:false);
		return flag;
	}
	
	public boolean timescale_less(String var, String ind){
		boolean flag = false;
		if(ind.equals("day")){
			if(var.equals("hour")||var.equals("minute")){
				flag = true;
			}
		}else if(ind.equals("hour")){
			if(var.equals("minute")){
				flag = true;
			}
		}else if(ind.equals("month")){
			if(var.equals("day")||var.equals("hour")||var.equals("minute")){
				flag = true;
			}
		}else if(ind.equals("year")){
			if(var.equals("month")||var.equals("day")||var.equals("hour")||var.equals("minute")){
				flag = true;
			}
		}
		return flag;
	}
	
	public boolean timescale_noless(String var, String ind){
		boolean flag = false;
		if(ind.equals("day")){
			if(var.equals("year")||var.equals("month")||var.equals("day")){
				flag = true;
			}
		}else if(ind.equals("hour")){
			if(var.equals("year")||var.equals("month")||var.equals("day")||var.equals("hour")){
				flag = true;
			}
		}else if(ind.equals("month")){
			if(var.equals("year")||var.equals("month")){
				flag = true;
			}
		}else if(ind.equals("year")){
			if(var.equals("year")){
				flag = true;
			}
		}
		return flag;
	}

}

