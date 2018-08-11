function v = helics_terminated()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 5);
  end
  v = vInitialized;
end
