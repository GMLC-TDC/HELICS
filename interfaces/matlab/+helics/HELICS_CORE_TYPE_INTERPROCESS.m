function v = HELICS_CORE_TYPE_INTERPROCESS()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230915);
  end
  v = vInitialized;
end
