describe("Tundra-Vector3df", function() {
	var precision = 0.00001;
	
	var x1 = Math.random()*100;
	var y1 = Math.random()*100;
	var z1 = Math.random()*100;
	
	var x2 = Math.random()*100;
	var y2 = Math.random()*100;
	var z2 = Math.random()*100;
	
	describe("Constructor", function() {
		
		it("Default constructor", function() {
			var v = new Vector3df();
			expect(Math.abs(v.x)).toBeLessThan(precision);
			expect(Math.abs(v.y)).toBeLessThan(precision);
			expect(Math.abs(v.z)).toBeLessThan(precision);
		});
	
		it("Correct float arguments constructor", function() {
			var v = new Vector3df(x1,y1,z1);
			expect(Math.abs(v.x - x1)).toBeLessThan(precision);
			expect(Math.abs(v.y - y1)).toBeLessThan(precision);
			expect(Math.abs(v.z - z1)).toBeLessThan(precision);
		});
		
		it("Incorrect arguments number constructor", function() {
			var e = new Error("Error creating Vector3df. " +
					"Usage: new Vector3df() or new Vector3df(x, y, z).");
			expect(function() {new Vector3df(x1,y1,z1,x2);}).toThrow(e);
		});

		it("Incorrect arguments type constructor", function() {
			var e = new TypeError("new Vector3df(x,y,z): z argument is not a number");
			expect(function() {new Vector3df(x1,y1, new Object());}).toThrow(e);
		});
		
		it("Incorrect call of function as constructor", function() {
			var e = new Error("Error creating Vector3df. " +
					"Usage: new Vector3df() or new Vector3df(x, y, z).");
			expect(function() {Vector3df(x1,y1,z1);}).toThrow(e);
		});
		
		it("Incorrect call of function as constructor", function() {
			var e = new Error("Error creating Vector3df. " +
					"Usage: new Vector3df() or new Vector3df(x, y, z).");
			expect(function() {Vector3df();}).toThrow(e);
		});
	
	});
	
	it("Add", function() {
		var v1 = new Vector3df(x1,y1,z1);
		var v2 = new Vector3df(x2,y2,z2);
		var v = v1.add(v2);
		expect(Math.abs(v.x - (v1.x + v2.x))).toBeLessThan(precision);
		expect(Math.abs(v.y - (v1.y + v2.y))).toBeLessThan(precision);
		expect(Math.abs(v.z - (v1.z + v2.z))).toBeLessThan(precision);
	});
	
	it("Subtract", function() {
		var v1 = new Vector3df(x1,y1,z1);
		var v2 = new Vector3df(x2,y2,z2);
		var v = v1.subtract(v2);
		expect(Math.abs(v.x - (v1.x - v2.x))).toBeLessThan(precision);
		expect(Math.abs(v.y - (v1.y - v2.y))).toBeLessThan(precision);
		expect(Math.abs(v.z - (v1.z - v2.z))).toBeLessThan(precision);
	});

	it("DotProduct", function() {
		var v1 = new Vector3df(x1,y1,z1);
		var v2 = new Vector3df(x2,y2,z2);
		var f = v1.dotProduct(v2);
		expect(Math.abs(f - (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z))).toBeLessThan(100*precision);
	});
	
	it("CrossProduct", function() {
		var v1 = new Vector3df(x1,y1,z1);
		var v2 = new Vector3df(x2,y2,z2);
		var v = v1.crossProduct(v2);
		expect(Math.abs(v.x - (v1.y * v2.z - v1.z * v2.y))).toBeLessThan(100*precision);
		expect(Math.abs(v.y - (v1.z * v2.x - v1.x * v2.z))).toBeLessThan(100*precision);
		expect(Math.abs(v.z - (v1.x * v2.y - v1.y * v2.x))).toBeLessThan(100*precision);
	});

	it("GetLength", function() {
		var v = new Vector3df(x1,y1,z1);
		var l = Math.sqrt(x1*x1 + y1*y1 + z1*z1);
		expect(Math.abs(v.getLength() - l)).toBeLessThan(precision);
	});
	
	it("Normalize", function() {
		var v = new Vector3df(x1,y1,z1);
		var l = Math.sqrt(x1*x1 + y1*y1 + z1*z1);
		if (l == 0)
			l = 1;
		v.normalize();
		expect(Math.abs(v.x - x1/l)).toBeLessThan(precision);
		expect(Math.abs(v.y - y1/l)).toBeLessThan(precision);
		expect(Math.abs(v.z - z1/l)).toBeLessThan(precision);
	});
});		
	