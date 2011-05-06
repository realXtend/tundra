angleDiff = function(ang1, ang2, precision) {
	var diff = Math.abs(ang1 - ang2);
	if (Math.abs(diff - 2*Math.PI) < precision)
		diff = Math.abs(diff - 2*Math.PI);
	return diff;
};


describe("Tundra-Quaternion", function() {
	var precision = 0.00001;

	var x1 = Math.random()*100;
	var y1 = Math.random()*100;
	var z1 = Math.random()*100;
	var w1 = Math.random()*100;
	
	var x2 = Math.random()*100;
	var y2 = Math.random()*100;
	var z2 = Math.random()*100;
	var w2 = Math.random()*100;
	
	describe("Constructor", function() {
		
		it("Default constructor", function() {
			var q = new Quaternion();
			expect(Math.abs(q.x)).toBeLessThan(precision);
			expect(Math.abs(q.y)).toBeLessThan(precision);
			expect(Math.abs(q.z)).toBeLessThan(precision);
			expect(Math.abs(q.w - 1)).toBeLessThan(precision);
		});
		
		it("Correct float arguments constructor", function() {
			var q = new Quaternion(x1, y1, z1, w1);
			expect(Math.abs(q.x - x1)).toBeLessThan(precision);
			expect(Math.abs(q.y - y1)).toBeLessThan(precision);
			expect(Math.abs(q.z - z1)).toBeLessThan(precision);
			expect(Math.abs(q.w - w1)).toBeLessThan(precision);
		});
		
		it("Correct euler vector constructor", function() {
			var v = new Vector3df();
			var q = new Quaternion(v);
			expect(Math.abs(q.x)).toBeLessThan(precision);
			expect(Math.abs(q.y)).toBeLessThan(precision);
			expect(Math.abs(q.z)).toBeLessThan(precision);
			expect(Math.abs(q.w - 1)).toBeLessThan(precision);
		});
		
		it("Incorrect arguments number constructor", function() {
			var e = new Error("Error creating Quaternion. Usage: new Quaternion(), " +
					"new Quaternion(v) or new Quaternion(x, y, z, w).");
			expect(function() {new Quaternion(x1, y1, z1);}).toThrow(e);
		});
		
		it("Incorrect arguments type constructor", function() {
			var e1 = new TypeError("new Quaternion(x,y,z,w): w argument is not a number");
			expect(function() {new Quaternion(x1, y1, z1, new Object());}).toThrow(e1);
			var e2 = new TypeError("new Quaternion(v): v argument is not a Vector3df.");
			expect(function() {new Quaternion(new Object());}).toThrow(e2);
		});
		
		it("Incorrect call of function as constructor", function() {
			var e = new Error("Error creating Quaternion. Usage: new Quaternion(), " +
			"new Quaternion(v) or new Quaternion(x, y, z, w).");
			expect(function() {Quaternion();}).toThrow(e);
			expect(function() {Quaternion(new Vector3df());}).toThrow(e);
			expect(function() {Quaternion(x1,y1,z1,w1);}).toThrow(e);
		});
	});
	
	describe("Quaternion Product", function() {
		it("2*3 = 6", function() {
			var q1 = new Quaternion(0,0,0,2);
			var q2 = new Quaternion(0,0,0,3);
			var q = q2.product(q1);
			expect(q.x).toEqual(0);
			expect(q.y).toEqual(0);
			expect(q.z).toEqual(0);
			expect(q.w).toEqual(6);
		});
		it("i*i = -1", function() {
			var q1 = new Quaternion(1,0,0,0);
			var q2 = new Quaternion(1,0,0,0);
			var q = q2.product(q1);
			expect(q.x).toEqual(0);
			expect(q.y).toEqual(0);
			expect(q.z).toEqual(0);
			expect(q.w).toEqual(-1);
		});
		it("j*j = -1", function() {
			var q1 = new Quaternion(0,1,0,0);
			var q2 = new Quaternion(0,1,0,0);
			var q = q2.product(q1);
			expect(q.x).toEqual(0);
			expect(q.y).toEqual(0);
			expect(q.z).toEqual(0);
			expect(q.w).toEqual(-1);
		});
		it("k*k = -1", function() {
			var q1 = new Quaternion(0,0,1,0);
			var q2 = new Quaternion(0,0,1,0);
			var q = q2.product(q1);
			expect(q.x).toEqual(0);
			expect(q.y).toEqual(0);
			expect(q.z).toEqual(0);
			expect(q.w).toEqual(-1);
		});
		it("i*j = k", function() {
			var q1 = new Quaternion(1,0,0,0);
			var q2 = new Quaternion(0,1,0,0);
			var q = q2.product(q1);
			expect(q.x).toEqual(0);
			expect(q.y).toEqual(0);
			expect(q.z).toEqual(1);
			expect(q.w).toEqual(0);
		});
		it("j*k = i", function() {
			var q1 = new Quaternion(0,1,0,0);
			var q2 = new Quaternion(0,0,1,0);
			var q = q2.product(q1);
			expect(q.x).toEqual(1);
			expect(q.y).toEqual(0);
			expect(q.z).toEqual(0);
			expect(q.w).toEqual(0);
		});
	});
	
	xit("DotProduct", function() {
		var q1 = new Quaternion(x1, y1, z1, w1);
		var q2 = new Quaternion(x2, y2, z2, w2);
		var f = q1.dotProduct(q2);
		expect(f).toEqual(q1.w * q2.w + q1.x * q2.x + 
				q1.y * q2.y + q1.z * q2.z);
	});
	
	describe("Euler conversion", function() {
		it("yaw", function() {
			var v = new Vector3df(Math.PI,0,Math.PI);
			var q = new Quaternion(v);
			expect(Math.abs(q.x)).toBeLessThan(precision);
			expect(Math.abs(q.y - 1)).toBeLessThan(precision);
			expect(Math.abs(q.z)).toBeLessThan(precision);
			expect(Math.abs(q.w)).toBeLessThan(precision);
			
			var v_q = q.toEuler();
			expect(angleDiff(v_q.x , v.x, precision)).toBeLessThan(precision);
			expect(angleDiff(v_q.y , v.y, precision)).toBeLessThan(precision);
			expect(angleDiff(v_q.z , v.z, precision)).toBeLessThan(precision);
		});
		it("roll", function() {
			var v = new Vector3df(Math.PI,0,0);
			var q = new Quaternion(v);
			expect(Math.abs(q.x - 1)).toBeLessThan(precision);
			expect(Math.abs(q.y)).toBeLessThan(precision);
			expect(Math.abs(q.z)).toBeLessThan(precision);
			expect(Math.abs(q.w)).toBeLessThan(precision);
			var v_q = q.toEuler();
			expect(angleDiff(v_q.x , v.x, precision)).toBeLessThan(precision);
			expect(angleDiff(v_q.y , v.y, precision)).toBeLessThan(precision);
			expect(angleDiff(v_q.z , v.z, precision)).toBeLessThan(precision);
		});
		it("pitch", function() {
			var v = new Vector3df(0,0,Math.PI);
			var q = new Quaternion(v);
			expect(Math.abs(q.x)).toBeLessThan(precision);
			expect(Math.abs(q.y)).toBeLessThan(precision);
			expect(Math.abs(q.z - 1)).toBeLessThan(precision);
			expect(Math.abs(q.w)).toBeLessThan(precision);
			var v_q = q.toEuler();
			expect(angleDiff(v_q.x , v.x, precision)).toBeLessThan(precision);
			expect(angleDiff(v_q.y , v.y, precision)).toBeLessThan(precision);
			expect(angleDiff(v_q.z , v.z, precision)).toBeLessThan(precision);
		});
	});
	
	it("Normalize", function() {
		var q = new Quaternion(x1, y1, z1, w1);
		q.normalize();
		expect(Math.abs(Math.sqrt(q.x * q.x + q.y * q.y +
				q.z * q.z + q.w * q.w) - 1)).toBeLessThan(precision);
	});
	
	it("Inverse", function() {
		var q = new Quaternion(x1, y1, z1, w1);//.normalize();
		var q_inv = new Quaternion(x1, y1, z1, w1);//.normalize();
		q_inv.makeInverse();
		var q_ide = q.product(q_inv);
		expect(Math.abs(q_ide.x)).toBeLessThan(precision);
		expect(Math.abs(q_ide.y)).toBeLessThan(precision);
		expect(Math.abs(q_ide.z)).toBeLessThan(precision);
		expect(Math.abs(q_ide.w - 1)).toBeLessThan(precision);
	});
	
	it("RotationFromTo", function() {
		var v1 = new Vector3df(x1,y1,z1).normalize();
		var v2 = new Vector3df(x2,y2,z2).normalize();
		var q = Quaternion.prototype.rotationFromTo(v1,v2);
		var v = q.product(v1);
		expect(Math.abs(v.x - v2.x)).toBeLessThan(100*precision);
		expect(Math.abs(v.y - v2.y)).toBeLessThan(100*precision);
		expect(Math.abs(v.z - v2.z)).toBeLessThan(100*precision);
	});
	
	
});