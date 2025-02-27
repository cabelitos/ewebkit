description('Test setting valid and invalid properties of HTMLMeterElement.');

var m = document.createElement('meter');

debug("Test values before properties were set");
shouldBe("m.min", "0");
shouldBe("m.value", "0");
shouldBe("m.max", "1");
shouldBe("m.low", "0");
shouldBe("m.high", "1");
shouldBe("m.optimum", "0.5");

debug("Set valid values");
m.min = "-10";
m.value = 7e1;
m.max = "1e2";
m.low = "10.1";
m.high = "99.5";
m.optimum = "70";
shouldBe("m.min", "-10");
shouldBe("m.value", "70");
shouldBe("m.max", "100");
shouldBe("m.low", "10.1");
shouldBe("m.high", "99.5");
shouldBe("m.optimum", "70");

debug("Set attributes to improper values - 1");
m.min = -10;
m.value = 200;
m.max = 100.0;
m.low = 200;
m.high = -50;
m.optimum = null;
shouldBe("m.min", "-10");
shouldBe("m.value", "100");
shouldBe("m.max", "100");
shouldBe("m.low", "100");
shouldBe("m.high", "100");
shouldBe("m.optimum", "0");

debug("Set attributes to improper values - 2");
m.min = 200.0;
m.value = -200.0;
m.max = 0;
m.low = null;
shouldBe("m.min", "200.0");
shouldBe("m.value", "200.0");
shouldBe("m.max", "200.0");
shouldBe("m.low", "200.0");

debug("Set attributes to improper values - 3");
m.min = 100.0;
m.value = 200.0;
m.max = 50;
m.low = 10;
m.high = 15e1;
m.optimum = 12.5;
shouldBe("m.min", "100.0");
shouldBe("m.value", "100.0");
shouldBe("m.max", "100.0");
shouldBe("m.low", "100.0");
shouldBe("m.high", "100.0");
shouldBe("m.optimum", "100.0");

debug("Set attributes to improper values - 4");
m.min = 0.0;
m.value = 250.0;
m.max = 200;
m.low = -10;
m.high = 15e2;
m.optimum = 12.5;
shouldBe("m.min", "0.0");
shouldBe("m.value", "200.0");
shouldBe("m.max", "200.0");
shouldBe("m.low", "0.0");
shouldBe("m.high", "200.0");
shouldBe("m.optimum", "12.5");

debug("Set value to invalid value");
shouldThrowErrorName('m.value = "value";', 'TypeError');

debug("Set min to NaN");
shouldThrowErrorName('m.min = NaN;', 'TypeError');

debug("Set max to Infinity");
shouldThrowErrorName('m.max = Infinity;', 'TypeError');

debug("Set low to invalid value");
shouldThrowErrorName('m.low = "low";', 'TypeError');

debug("Set high to NaN");
shouldThrowErrorName('m.high = NaN;', 'TypeError');

debug("Set optimum to Infinity");
shouldThrowErrorName('m.optimum = Infinity;', 'TypeError');

debug("Set attributes to valid numbers");
m.setAttribute("min", 0);
m.setAttribute("value", 5);
m.setAttribute("max", 10);
shouldBe("m.value", "5");
shouldBe("m.max", "10");
shouldBe("parseInt(m.getAttribute('value'))", "5");
shouldBe("parseInt(m.getAttribute('max'))", "10");

debug("Set attributes to invalid values");
m.setAttribute("value", "ABC");
m.setAttribute("max", "#");
shouldBe("m.value", "0");
shouldBe("m.max", "1");
shouldBe("m.getAttribute('value')", "'ABC'");
shouldBe("m.getAttribute('max')", "'#'");

debug("Set attributes to numbers with leading spaces");
m.setAttribute("value", " 5");
m.setAttribute("min", " 5");
m.setAttribute("max", " 5");
m.setAttribute("low", " 5");
m.setAttribute("high", " 5");
m.setAttribute("optimum", " 5");
shouldBe("m.value", "0");
shouldBe("m.min", "0");
shouldBe("m.max", "1");
shouldBe("m.low", "0");
shouldBe("m.high", "1");
shouldBe("m.optimum", "0.5");

