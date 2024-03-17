import sys
import tests.ble 

def print_help():
    print("\nUsage: python -m tests.ble [comport]")
    sys.exit(1)

def main():
    if len(sys.argv) < 2:
        print_help()

    comport = sys.argv[1]
    module = __import__(tests.ble.__name__, fromlist=["test"])
    example = getattr(module, "test")
    example.main(comport)

if __name__ == '__main__':
    main()
