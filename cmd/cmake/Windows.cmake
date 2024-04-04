set(QT_LIBS
  ${QT_PREFIX}::Core
  ${QT_PREFIX}::Network
  ${QT_PREFIX}::SerialPort)

foreach(QT_LIB IN ITEMS ${QT_LIBS})
  get_target_property(LIBRARY_PATH ${QT_LIB} LOCATION)
  install(FILES
      "${LIBRARY_PATH}"
      DESTINATION "${DLT_EXECUTABLE_INSTALLATION_PATH}"
      COMPONENT qt_libraries)
endforeach()