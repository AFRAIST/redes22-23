import angr
import claripy
import sys

def data_to_arr(_s1):
    s1_dec = []
    for i in range(0x29):
        s1_dec += [((_s1 >> ((i) << 3)) & 0xff)]
    return s1_dec

def main():
    proj = angr.Project("./player")
  
    addr = proj.loader.find_symbol("process_buffer.constprop.0").rebased_addr

    s1 = 0x69000000
    s2 = s1 + 0x29

    first_state = proj.factory.call_state(addr, s1, s2, prototype="char f(char *buf, void *st)")

    ss = b"a b\n\x00"
    while ss != b"":
        ss += b"\x00"*(0x29 - len(ss))
        assert len(ss) == 0x29

        ss = int.from_bytes(ss, byteorder="big")
        _s1 = claripy.BVV(ss, 0x29 * 8)
        
        """ Symbolic is a nono
        _s1 = claripy.BVS('a', 0x29 * 8)
        s1_dec = data_to_arr(_s1)

        first_state.solver.add(claripy.Or(*[x == 0 for x in s1_dec]))
        first_state.solver.add(claripy.Or(*[x == ord(' ') for x in s1_dec]))
        first_state.solver.add(claripy.Or(*[x == ord('\n') for x in s1_dec]))
        first_state.solver.add(claripy.Or(*[x != 0 for x in s1_dec]))
        first_state.solver.add(s1_dec[-1] == 0)
        """ 


        _s2 = claripy.BVS('b', 0x100 * 8)

        first_state.memory.store(s1, _s1)
        first_state.memory.store(s2, _s2)

        sim = proj.factory.simgr(first_state)
        find = 0x4015D5
        @proj.hook(0x400000+0x155c, length=0x1580-0x155c)
        def hook(state):
            pass

        sim.explore(find=find, avoid=0x401574)
        if sim.found:
            print(sim.found[0].regs.rax)
        else:
            print("No stuff")

        #import IPython; IPython.embed()
        ss = eval(input())
    #first_state = proj.factory.call_state()

    

if __name__ == "__main__":
    sys.exit(main())

