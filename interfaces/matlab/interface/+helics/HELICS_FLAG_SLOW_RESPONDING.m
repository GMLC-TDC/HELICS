function v = HELICS_FLAG_SLOW_RESPONDING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 44);
  end
  v = vInitialized;
end
