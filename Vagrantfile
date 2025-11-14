Vagrant.configure("2") do |config|
    config.vm.box = "bento/ubuntu-22.04"
    config.vm.hostname = "MaqCompilador"
    config.vm.provider "virtualbox" do |vb|
    config.vm.synced_folder "../compiler-project/", "/src/compilador"
    vb.gui = false
    vb.memory = "4096"
    vb.cpus = 4
    vb.name = "MaqCompilador"
    end
    end