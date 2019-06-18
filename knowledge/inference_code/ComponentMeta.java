package inferEngine;

import java.util.HashSet;

public class ComponentMeta {
	
	public String algorithmName;
	public String componentName;
	public HashSet<String> inputNames = new HashSet<String>();
	public HashSet<String> outputNames = new HashSet<String>();
	public HashSet<String> optionalInputNames = new HashSet<String>();
	
}