#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint flutter_silk.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'flutter_silk'
  s.version          = '0.0.1'
  s.summary          = 'A flutter ffi plugin for converting audio from silk to pcm/mp3.'
  s.description      = <<-DESC
A flutter ffi plugin for converting audio from silk to pcm/mp3.
                       DESC
  s.homepage         = 'https://github.com/WebbingTech/flutter_silk'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Webbing Tech' => 'cillywill05@gmail.com' }

  # This will ensure the source files in Classes/ are included in the native
  # builds of apps using this FFI plugin. Podspec does not support relative
  # paths, so Classes contains a forwarder C file that relatively imports

  s.prepare_command = <<-CMD
    rsync -a --delete ../src/  Classes
  CMD

  s.source           = { :path => '.' }
  s.source_files     = [
    'Classes/*.c',
    'Classes/silk/src/*.c',
    'Classes/lame/*.c',
    'Classes/lame/libmp3lame/*.c',
    'Classes/lame/libmp3lame/vector/*.c',
    'Classes/lame/mpglib/*.c'
  ]
  s.dependency 'Flutter'

  s.platform = :ios, '11.0'
  s.pod_target_xcconfig = {
    'OTHER_CFLAGS' => '-DDEBUG=0',
    'OTHER_LDFLAGS' => '-Wl,-v',
    'DEFINES_MODULE' => 'YES',
    'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'i386',
    'HEADER_SEARCH_PATHS' => [
      '"${PODS_TARGET_SRCROOT}/Classes"',
      '"${PODS_TARGET_SRCROOT}/Classes/silk/interface"',
      '"${PODS_TARGET_SRCROOT}/Classes/silk/src"',
      '"${PODS_TARGET_SRCROOT}/Classes/lame"',
      '"${PODS_TARGET_SRCROOT}/Classes/lame/include"',
      '"${PODS_TARGET_SRCROOT}/Classes/lame/libmp3lame"',
      '"${PODS_TARGET_SRCROOT}/Classes/lame/mpglib"'
    ],
    'GCC_PREPROCESSOR_DEFINITIONS' => [
      '$(inherited) DEBUG=0',
      'HAVE_STDINT_H',
      'HAVE_MPGLIB',
      'DECODE_ON_THE_FLY',
      'USE_FAST_LOG',
      'TAKEHIRO_IEEE754_HACK',
      'STDC_HEADERS',
      'ieee754_float32_t=float'
    ]
  }
  s.swift_version = '5.0'
end
