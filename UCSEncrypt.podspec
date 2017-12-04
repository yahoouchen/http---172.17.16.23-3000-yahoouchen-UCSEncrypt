Pod::Spec.new do |s|
s.name         = "UCSEncrypt"
s.version      = "1.1.0"
s.summary      = "UCSEncrypt"
s.homepage     = "http://172.17.16.23:3000/chenxiancai/UCSEncrypt"
s.license      = { :type => "MIT", :file => "LICENSE" }
s.author        = { "xiexy" => "xie.xiaoyong@ucsmy.com" }
s.platform     = :ios, "7.0"
s.source       = { :git => "http://172.17.16.23:3000/chenxiancai/UCSEncrypt.git", :tag => s.version }
s.source_files = "UCSEncrypt/Lib/*.{h,m,c}"
s.requires_arc = true
s.frameworks   = 'Foundation'
s.libraries =  'stdc++','z'
s.ios.vendored_frameworks = 'UCSEncrypt/Lib/openssl.framework'
s.dependency "UCSBase64"
end

