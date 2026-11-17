"""
Demonstration and validation of C3 Linearization (MRO) and cooperative super()
in Diamond Multiple Inheritance hierarchies.
"""

class BaseA:
    def __init__(self, trace: list):
        trace.append("BaseA")

class LeftB(BaseA):
    def __init__(self, trace: list):
        trace.append("LeftB_start")
        super().__init__(trace)
        trace.append("LeftB_end")

class RightC(BaseA):
    def __init__(self, trace: list):
        trace.append("RightC_start")
        super().__init__(trace)
        trace.append("RightC_end")

class BottomD(LeftB, RightC):
    def __init__(self, trace: list):
        trace.append("BottomD_start")
        super().__init__(trace)
        trace.append("BottomD_end")

def test_mro_order():
    expected_mro = [BottomD, LeftB, RightC, BaseA, object]
    actual_mro = list(BottomD.__mro__)
    assert actual_mro == expected_mro, f"MRO mismatch: {actual_mro} != {expected_mro}"
    print(f"[PASS] MRO Verified: {[cls.__name__ for cls in actual_mro]}")

def test_super_execution_trace():
    trace = []
    BottomD(trace)
    expected_trace = [
        "BottomD_start",
        "LeftB_start",
        "RightC_start",
        "BaseA",
        "RightC_end",
        "LeftB_end",
        "BottomD_end"
    ]
    assert trace == expected_trace, f"Trace mismatch: {trace} != {expected_trace}"
    print(f"[PASS] Cooperative super() trace verified: {' -> '.join(trace)}")

if __name__ == "__main__":
    test_mro_order()
    test_super_execution_trace()
