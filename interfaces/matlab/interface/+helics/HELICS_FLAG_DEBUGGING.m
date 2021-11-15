function v = HELICS_FLAG_DEBUGGING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 50);
  end
  v = vInitialized;
end
