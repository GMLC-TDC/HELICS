function v = helics_terminated()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107592);
  end
  v = vInitialized;
end
