package engineUtility;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class InvokeMethod {

	 public static  boolean  invoke(String _rel, String _var,String _ind) throws InstantiationException, IllegalAccessException, SecurityException, NoSuchMethodException, IllegalArgumentException, InvocationTargetException{
			
	    	String rel = null;
	    	String var = _var;
	    	String ind = null;
	    	
	    	if(_ind == null){
	    	   if(_rel.contains("is ")){
	    		   rel = "is";
	    		   ind = _rel.split("is ")[1];
	    	   }
	    	   if(_rel.contains("are ")){
	    		   rel = "are";
	    		   ind = _rel.split("are ")[1];
	    	   }
	    	}else{
	    		rel = _rel;
	    		ind = _ind;
	    	}
	    	
	    	Class utility = CommonUtility.class;
			Object obj = utility.newInstance();
			Method method = utility.getMethod(rel, new Class[]{String.class,String.class});
			boolean result = (Boolean)method.invoke(obj, new Object[]{var,ind});
			return result;
		}
	    
}
