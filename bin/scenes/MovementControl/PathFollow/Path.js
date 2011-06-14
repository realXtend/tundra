/**
 * @name PathFollow/Path.js
 * @fileOverview Calculates the next point in a Path.
 * <dl>
 * <dt><b>Usage: </b>Include the script to make use of methods:</dt>
 * <dd>&bull; {@link realXtend.PathFollow.getNextWaypoint}</dd> 
 * </dl>
 */

/**
 * @namespace main namespace.
 */
var realXtend = realXtend || {};

/**
 * @namespace PathFollow namespace contains functionality to follow paths. 
 */
realXtend.PathFollow = realXtend.PathFollow || {};


/**
 * Calculates next waypoint and direction of the path.
 * @param pathid Id of the Path.
 * @param actualpointid Id of the previous point.
 * @param direction Direction of the Path.
 * @returns {Array} [NextPoint,Direction].
 * @static
 */
realXtend.PathFollow.getNextWaypoint = function(pathid,actualpointid,direction)
{	
	var path=scene.GetEntityRaw(pathid);
	var dynamiccomponent=path.GetComponentRaw("EC_DynamicComponent");
	var list=dynamiccomponent.GetAttribute("WayPointList");
	var elem = list.split(',');	
	var pointnum = elem.length;	
	var random=dynamiccomponent.GetAttribute("Random");
	var type=dynamiccomponent.GetAttribute("Type");	
	var elemi;
	if(actualpointid!=0){
		var actualpoint=scene.GetEntityRaw(actualpointid);
		if(random==false){
			if(type==1){
				for (i=0;i<pointnum;i++){
					var where=scene.GetEntityByNameRaw("WayPoint["+elem[i]+"]");
					if(typeof(where) != "undefined" && where.id==actualpointid){
						if(direction==1){
							elemi=scene.GetEntityByNameRaw("WayPoint["+elem[pointnum-1]+"]");
							if(typeof(elemi) != "undefined" && where.id==elemi.id){
								return [0,direction];
							}
							else if(typeof(elemi) != "undefined" && where.id!=elemi.id){
								var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[i+1]+"]");
								return [nextpoint.id,direction];
							}
						}
						else if(direction==-1){
							elemi=scene.GetEntityByNameRaw("WayPoint["+elem[0]+"]");
							if(typeof(elemi) != "undefined" && where.id==elemi.id){
								return [0,direction];
							}
							else if(typeof(elemi) != "undefined" && where.id!=elemi.id){
								var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[i-1]+"]");
								return [nextpoint.id,direction];
							}
						}
					}
				}
			}
			else if(type==2){
				for (i=0;i<pointnum;i++){
					var where=scene.GetEntityByNameRaw("WayPoint["+elem[i]+"]");
					if(typeof(where) != "undefined" && where.id==actualpointid){
						if(direction==1){
							elemi=scene.GetEntityByNameRaw("WayPoint["+elem[pointnum-1]+"]");
							if(typeof(elemi) != "undefined" && where.id==elemi.id){
								var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[i-1]+"]");
								direction=-1;
								return [nextpoint.id,direction];
							}
							else if(typeof(elemi) != "undefined" && where.id!=elemi.id){
								var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[i+1]+"]");
								return [nextpoint.id,direction];
							}
						}
						else if(direction==-1){
							elemi=scene.GetEntityByNameRaw("WayPoint["+elem[0]+"]");
							if(typeof(elemi) != "undefined" && where.id==elemi.id){
								var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[1]+"]");
								direction=1;
								return [nextpoint.id,direction];
							}
							else if(typeof(elemi) != "undefined" && where.id!=elemi.id){
								var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[i-1]+"]");
								return [nextpoint.id,direction];
							}
						}
					}
				}
			}
			else if(type==3){
				for (i=0;i<pointnum;i++){
					var where=scene.GetEntityByNameRaw("WayPoint["+elem[i]+"]");
					if(typeof(where) != "undefined" && where.id==actualpointid){
						if(direction==1){
							elemi=scene.GetEntityByNameRaw("WayPoint["+elem[pointnum-1]+"]");
							if(typeof(elemi) != "undefined" && where.id==elemi.id){
								var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[0]+"]");
								return [nextpoint.id,direction];
							}
							else if(typeof(elemi) != "undefined" && where.id!=elemi.id){
								var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[i+1]+"]");
								return [nextpoint.id,direction];
							}
						}
						else if(direction==-1){
							elemi=scene.GetEntityByNameRaw("WayPoint["+elem[0]+"]");
							if(typeof(elemi) != "undefined" && where.id==elemi.id){
								var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[pointnum-1]+"]");
								return [nextpoint.id,direction];
							}
							else if(typeof(elemi) != "undefined" && where.id!=elemi.id){
								var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[i-1]+"]");
								return [nextpoint.id,direction];
							}
						}
					}
				}
			}
		}
		else if(random==true){
			var i = Math.floor(Math.random() * pointnum);
			var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[i]+"]");
				return [nextpoint.id,direction];
		}
	}
	else if(actualpointid==0){
		if(type==1 || type ==2){
			if(direction==1){
				var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[0]+"]");
				return [nextpoint.id,direction];
			}
			else if(direction==-1){
				var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[pointnum-1]+"]");
				return [nextpoint.id,direction];
			}
		}
		if(type==3){
			var nextpoint=scene.GetEntityByNameRaw("WayPoint["+elem[0]+"]");
			return [nextpoint.id,direction];
		}
	}
};