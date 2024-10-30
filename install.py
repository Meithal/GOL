import subprocess
import pathlib

def _install_glad():
    subprocess.check_call(["python", "-m", "venv", "venv"])
    subprocess.check_call(["chmod u+x ./venv/bin/activate*"], shell=True)
    subprocess.check_call(["./venv/bin/activate"], shell=True)
    subprocess.check_call(["python", "-m", "pip", "install", "glad"])
    subprocess.check_call(["glad", "--profile", "core", "--out-path",  "./glad", "--api", "gl=4.6", "--generator", "c-debug"])

def _install_glfw():
    subprocess.check_call(["git", "init"])
    subprocess.check_call(["git", "submodule", "add", "--depth", "1", "https://github.com/glfw/glfw", "glfw"])


def _main():
    if not (pathlib.Path('.') / 'glad').exists():
        _install_glad()
    if not (pathlib.Path('.') / 'glfw').exists():
        _install_glfw()

if __name__ == '__main__':
    _main()
